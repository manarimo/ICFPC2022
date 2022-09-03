import * as fsPromises from 'fs/promises';
import { PNG } from 'pngjs';
import { Move } from '../src/parser';
import { applySingleMove, createNewState, State } from '../src/simulate';
import { allSolutions, calculateScore, Image, loadMoves, loadProblem, Solution, solutionsForBatch, SolutionSpec } from './util';
import * as fs from 'fs';
import commandLineArgs from 'command-line-args';

interface Options {
    // Force recalculating everything
    force: boolean;

    // Run scorer only on specified batches
    only: string[];
}

function run(image: Image, solution: Move[]): State {
    let state = createNewState(image.width, image.height);
    solution.forEach((move, i) => {
        const res = applySingleMove(move, state);
        if (res.kind == 'error') {
            throw new Error(`Simulation failed at ${i}-th move: ${res.errorMessage}`);
        }
        state = res.state;
    });
    return state;
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

async function main(options: Options) {
    let solutionsByProblem: Record<string, Solution[]> = {};
    if (fs.existsSync(`../../output/ranking.json`)) {
        solutionsByProblem = JSON.parse(fs.readFileSync('../../output/ranking.json').toString());
    }

    let solutionSpecsIter: AsyncIterable<SolutionSpec>;
    if (options.only.length == 0) {
        solutionSpecsIter = allSolutions();
    } else {
        solutionSpecsIter = (async function* () {
            for (let batchName of options.only) {
                yield* solutionsForBatch(batchName);
            }
        })();
    }

    // Enumerate all batches
    for await (let spec of solutionSpecsIter) {
        if (!options.force && fs.existsSync(spec.scoreJsonPath())) {
            console.log(`Already processed: skip ${spec.solutionPath()}`);
            continue;
        }

        console.log(`../../output/${spec.batchName}/${spec.problemId}`);

        // Calculate score of the solution
        const problem = await loadProblem(spec.problemImagePath());
        const moves = await loadMoves(spec.solutionPath());
        const lastState = await run(problem, moves);
        const score = calculateScore(problem, lastState);

        // Write out the score to spec JSON
        await fsPromises.writeFile(spec.scoreJsonPath(), JSON.stringify({ score: score }));

        // Write out the last state image
        await writeSolutionImage(lastState, spec.solutionImagePath());

        // Record solution in the buffer
        const solution = new Solution(spec.batchName, spec.problemId, score);
        if (spec.problemId in solutionsByProblem) {
            if (solutionsByProblem[spec.problemId].every((s) => s.batchName != solution.batchName)) {
                solutionsByProblem[spec.problemId].push(solution);
            }
        } else {
            solutionsByProblem[spec.problemId] = [solution];
        }
    }

    // Sort solutions by ascending order of score
    for (let solutions of Object.values(solutionsByProblem)) {
        solutions.sort((a, b) => a.score - b.score);
    }

    // Fix the order of keys for consistent output
    const dict: typeof solutionsByProblem = {};
    for (let problemId of Object.keys(solutionsByProblem).sort()) {
        dict[problemId] = solutionsByProblem[problemId];
    }

    // Write out the summary JSON
    await fsPromises.writeFile(`../../output/ranking.json`, JSON.stringify(dict));
}

const options: Options = commandLineArgs([
    { name: 'force', alias: 'f', type: Boolean },
    { name: 'only', type: String, multiple: true },
]) as Options;
main(options);
