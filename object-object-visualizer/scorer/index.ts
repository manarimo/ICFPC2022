import * as fsPromises from 'fs/promises';
import { PNG } from 'pngjs';
import { Move } from '../src/parser';
import { applySingleMove, createNewState, State } from '../src/simulate';
import { calculateScore, dirEntries, Image, loadMoves, loadProblem, Solution } from './util';
import * as fs from 'fs';

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

// When force = true, process all solutions and updates scores.
// Otherwise, only processes solutions that don't have corresponding json file.
async function main(force: boolean) {
    let solutionsByProblem: Record<string, Solution[]> = {};
    if (fs.existsSync(`../../output/ranking.json`)) {
        solutionsByProblem = JSON.parse(fs.readFileSync('../../output/ranking.json').toString());
    }

    // Enumerate all batches
    for await (let entry of dirEntries('../../output')) {
        if (!entry.isDirectory()) {
            continue;
        }

        // Enumerate all solutions in the batch
        for await (let entry2 of dirEntries(`../../output/${entry.name}`)) {
            if (entry2.isDirectory()) {
                continue;
            }
            if (!entry2.name.endsWith('.isl')) {
                continue;
            }

            // Extract ID from the solution file
            const id = entry2.name.slice(0, -4);
            if (!force && fs.existsSync(`../../output/${entry.name}/${id}.json`)) {
                console.log(`Already processed: skip ../../output/${entry.name}/${entry2.name}`);
                continue;
            }

            console.log(`../../output/${entry.name}/${entry2.name}`);

            // Calculate score of the solution
            const problemPngFile = `../../problem/original/${id}.png`;
            const solutionFile = `../../output/${entry.name}/${entry2.name}`;
            const problem = await loadProblem(problemPngFile);
            const moves = await loadMoves(solutionFile);
            const lastState = await run(problem, moves);
            const score = calculateScore(problem, lastState);

            // Write out the score to spec JSON
            const specJsonFile = `../../output/${entry.name}/${id}.json`;
            await fsPromises.writeFile(specJsonFile, JSON.stringify({ score: score }));

            // Write out the last state image
            const solutionPngFile = `../../output/${entry.name}/${id}.png`;
            await writeSolutionImage(lastState, solutionPngFile);

            // Record solution in the buffer
            const solution = new Solution(entry.name, id, score);
            if (id in solutionsByProblem) {
                if (solutionsByProblem[id].every((s) => s.batchName != solution.batchName)) {
                    solutionsByProblem[id].push(solution);
                }
            } else {
                solutionsByProblem[id] = [solution];
            }
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

main(process.argv[2] == '--force');
