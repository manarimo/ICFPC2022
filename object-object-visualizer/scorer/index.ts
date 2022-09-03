import * as fsPromises from 'fs/promises';
import { PNG } from 'pngjs';
import { Move, parseProgram } from './parser';
import { State, createNewState, applySingleMove, calculateSimilarity } from './simulate';

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

async function main() {
    const problemPngFile = process.argv[2];
    const solutionFile = process.argv[3];

    const problem = await loadProblem(problemPngFile);
    const solution = await loadSolution(solutionFile);
    const solutionOutput = run(problem, solution);
    const score = calculateScore(problem, solutionOutput);

    console.log(score);
}

main();
