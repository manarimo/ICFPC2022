import * as fsPromises from 'fs/promises';
import { PNG } from 'pngjs';
import { Move, parseProgram } from '../src/parser';
import { State, createNewState, applySingleMove, calculateSimilarity } from '../src/simulate';

interface Image {
    r: Uint8Array;
    g: Uint8Array;
    b: Uint8Array;
    a: Uint8Array;
    width: number;
    height: number;
}

async function loadProblem(pngFile: string): Promise<Image> {
    const handle = await fsPromises.open(pngFile);
    const stream = await handle.createReadStream();
    const png = new PNG();
    const promise = new Promise<Image>((resolve, reject) => {
        png.on('parsed', function () {
            const numPx = png.width * png.height;
            const image: Image = {
                r: new Uint8Array(numPx),
                g: new Uint8Array(numPx),
                b: new Uint8Array(numPx),
                a: new Uint8Array(numPx),
                width: png.width,
                height: png.height,
            };
            for (let px = 0; px < numPx; px++) {
                const base = px * 4;
                image.r[px] = this.data[base + 0];
                image.g[px] = this.data[base + 1];
                image.b[px] = this.data[base + 2];
                image.a[px] = this.data[base + 3];
            }
            resolve(image);
        });
        png.on('error', function (err) {
            reject(err);
        });
    });
    stream.pipe(png);

    return promise;
}

async function loadSolution(solutionFile: string): Promise<Move[]> {
    const solutionBuffer = await fsPromises.readFile(solutionFile);
    return parseProgram(solutionBuffer.toString());
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

function calculateScore(problem: Image, state: State): number {
    return state.cost + calculateSimilarity(problem, state);
}

async function doScoring(problemPngFile: string, solutionFile: string): Promise<number> {
    const problem = await loadProblem(problemPngFile);
    const solution = await loadSolution(solutionFile);
    const solutionOutput = run(problem, solution);
    return calculateScore(problem, solutionOutput);
}

async function writeSolutionImage(state: State, destFile: string): Promise<void> {
    const png = new PNG({
        width: state.width,
        height: state.height,
    });
    const numPx = state.width * state.height;
    for (let px = 0; px < numPx; px++) {
        const pngX = px % state.width;
        const pngY = state.height - Math.floor(px / state.width) - 1;
        const pngPx = pngY * state.width + pngX;
        png.data[pngPx + 0] = state.r[px];
        png.data[pngPx + 1] = state.g[px];
        png.data[pngPx + 2] = state.b[px];
        png.data[pngPx + 3] = state.a[px];
    }

    const fileHandle = await fsPromises.open(destFile, 'w');
    return new Promise((resolve, reject) => {
        const packed = png.pack();
        packed.on('end', () => resolve());
        packed.on('error', (err) => reject(err));
        packed.pipe(fileHandle.createWriteStream());
    });
}

async function* dirEntries(path: string) {
    const dirHandle = await fsPromises.opendir(path);
    while(true) {
        const dir = await dirHandle.read();
        if (dir == null) {
            break;
        }
        yield dir;
    }
    dirHandle.close();
}

async function main() {
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
            console.log(`../../output/${entry.name}/${entry2.name}`);

            // Extract ID from the solution file
            const id = entry2.name.slice(0, -4);

            // Calculate score of the solution
            const problemPngFile = `../../problem/original/${id}.png`;
            const solutionFile = `../../output/${entry.name}/${entry2.name}`;
            const score = await doScoring(problemPngFile, solutionFile)

            // Write out the score to spec JSON
            const specJsonFile = `../../output/${entry.name}/${id}.json`;
            await fsPromises.writeFile(specJsonFile, JSON.stringify({score: score}));
        }
    }
}

main();