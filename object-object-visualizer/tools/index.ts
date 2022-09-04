import * as fsPromises from 'fs/promises';
import { PNG } from 'pngjs';
import { State } from '../src/simulate';
import { allSolutions, calculateScore, loadInitialBlocks, loadMoves, loadProblem, runSolution, solutionsForBatch, SolutionSpec } from './util';
import * as fs from 'fs';
import commandLineArgs from 'command-line-args';
import { buildRaking } from './ranker';

interface Options {
    // Force recalculating everything
    force?: boolean;

    // Run scorer only on specified batches
    only?: string[];
}

async function writeSolutionImage(state: State, destFile: string): Promise<void> {
    const png = new PNG({
        width: state.width,
        height: state.height,
        colorType: 6,
        inputColorType: 6,
        inputHasAlpha: true,
    });
    const numPx = state.width * state.height;
    for (let px = 0; px < numPx; px++) {
        const pngX = px % state.width;
        const pngY = state.height - Math.floor(px / state.width) - 1;
        const pngPx = (pngY * state.width + pngX) * 4;
        png.data[pngPx + 0] = state.r[px];
        png.data[pngPx + 1] = state.g[px];
        png.data[pngPx + 2] = state.b[px];
        png.data[pngPx + 3] = state.a[px];
    }

    const fileHandle = await fsPromises.open(destFile, 'w');
    return new Promise((resolve, reject) => {
        const packed = png.pack();
        packed.on('end', () => {
            fileHandle.close();
            resolve();
        });
        packed.on('error', (err) => {
            fileHandle.close();
            reject(err);
        });
        packed.pipe(fileHandle.createWriteStream());
    });
}

async function doScoring(spec: SolutionSpec) {
    console.log(`../../output/${spec.batchName}/${spec.problemId}`);

    // Calculate score of the solution
    const problem = await loadProblem(spec.problemImagePath());
    const moves = await loadMoves(spec.solutionPath());
    const initialBlocks = await loadInitialBlocks(spec.initialBlocksPath());
    const initialImage = fs.existsSync(spec.initialImagePath()) ? await loadProblem(spec.initialImagePath()) : undefined;
    const lastState = await runSolution(problem, moves, initialBlocks, initialImage);
    const score = calculateScore(problem, lastState);

    // Write out the score to spec JSON
    await fsPromises.writeFile(spec.scoreJsonPath(), JSON.stringify({ score: score }));

    // Write out the last state image
    await writeSolutionImage(lastState, spec.solutionImagePath());
}

async function main(options: Options) {
    let solutionSpecsIter: AsyncIterable<SolutionSpec>;
    if (options.only === undefined) {
        solutionSpecsIter = allSolutions();
    } else {
        solutionSpecsIter = (async function* () {
            for (let batchName of options.only!!) {
                yield* solutionsForBatch(batchName);
            }
        })();
    }

    // Calculate scores for specified batches (or all batches if --only is not given)
    for await (let spec of solutionSpecsIter) {
        if (!options.force && fs.existsSync(spec.scoreJsonPath())) {
            console.log(`Already processed: skip ${spec.solutionPath()}`);
            continue;
        }

        try {
            await doScoring(spec);
        } catch (e) {
            console.error(`Failed to scoring ${spec.batchName}/${spec.problemId}`, e);
        }
    }

    // Now all score files are guaranteed to be up-to-date.
    // Recalculate the ranking JSON.
    console.log('Rebuilding ranking.json...');
    const ranking = await buildRaking();
    await fsPromises.writeFile(`../../output/ranking.json`, JSON.stringify(ranking));
}

const options: Options = commandLineArgs([
    { name: 'force', alias: 'f', type: Boolean },
    { name: 'only', type: String, multiple: true },
]) as Options;
main(options);
