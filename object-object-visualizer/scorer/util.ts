import * as fsPromises from 'fs/promises';
import * as fs from 'fs';
import { PNG } from 'pngjs';
import { Move, parseProgram } from '../src/parser';
import { State, applySingleMove, calculateSimilarity, InitialBlock } from '../src/simulate';

export interface Image {
    r: Uint8Array;
    g: Uint8Array;
    b: Uint8Array;
    a: Uint8Array;
    width: number;
    height: number;
}

export class SolutionSpec {
    constructor(readonly batchName: string, readonly problemId: string) {}

    get solutionPath(): string {
        return `${this.batchName}/${this.problemId}.isl`;
    }
}

export class Solution {
    constructor(readonly batchName: string, readonly problemId: string, readonly score: number) {}
}

export async function loadProblem(pngFile: string): Promise<Image> {
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

export async function loadMoves(solutionFile: string): Promise<Move[]> {
    const solutionBuffer = await fsPromises.readFile(solutionFile);
    return parseProgram(solutionBuffer.toString());
}

export async function loadInitialBlocks(initialBlocksFile: string): Promise<InitialBlock[] | undefined> {
    if (!fs.existsSync(initialBlocksFile)) {
        return undefined;
    }
    const initialBlockBuffer = await fsPromises.readFile(initialBlocksFile);
    return JSON.parse(initialBlockBuffer.toString()).blocks as InitialBlock[];
}

export async function* dirEntries(path: string) {
    const dirHandle = await fsPromises.opendir(path);
    while (true) {
        const dir = await dirHandle.read();
        if (dir == null) {
            break;
        }
        yield dir;
    }
    dirHandle.close();
}

export async function* allSolutions(): AsyncGenerator<SolutionSpec> {
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

            const problemId = entry2.name.slice(0, -4);
            yield new SolutionSpec(entry.name, problemId);
        }
    }
}

export async function* topNSolutions(topN: number): AsyncGenerator<Solution> {
    const rankingJson = await fsPromises.readFile(`../../output/ranking.json`);
    const ranking = JSON.parse(rankingJson.toString()) as Record<string, Solution[]>;
    for (let solutions of Object.values(ranking)) {
        for (let solution of solutions.slice(0, topN)) {
            yield solution;
        }
    }
}

export function calculateScore(problem: Image, state: State): number {
    return state.cost + calculateSimilarity(problem, state);
}
