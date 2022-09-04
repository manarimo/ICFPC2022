import { Move, parseProgram } from '../../src/parser';
import * as fsPromises from 'fs/promises';
import * as fs from 'fs';
import { Image, loadInitialBlocks, loadMoves, loadProblem } from '../util';
import { applySingleMove, calculatePixelSimilarity, createNewState, State } from '../../src/simulate';

const calcSinglePixelDistance = (problem: Image, color: Color, { r, c }: Place): number => {
    const px = (problem.height - r - 1) * problem.width + c;

    const rDiff = problem.r[px] - color.r;
    const gDiff = problem.g[px] - color.g;
    const bDiff = problem.b[px] - color.b;
    const aDiff = problem.a[px] - color.a;
    return Math.sqrt(rDiff * rDiff + gDiff * gDiff + bDiff * bDiff + aDiff * aDiff);
};

const calcSimilarityOfPlaces = (problem: Image, color: Color, places: Place[]) => {
    let sum = 0.0;
    places.forEach((place) => {
        sum += calcSinglePixelDistance(problem, color, place);
    });
    return sum;
};

const optimizeColor = (problem: Image, places: Place[]) => {
    let rMin = 255;
    let rMax = 0;
    let gMin = 255;
    let gMax = 0;
    let bMin = 255;
    let bMax = 0;
    let aMin = 255;
    let aMax = 0;
    places.forEach(({ r, c }) => {
        rMin = Math.min(rMin, problem.r[r * problem.height + c]);
        rMax = Math.max(rMax, problem.r[r * problem.height + c]);
        gMin = Math.min(gMin, problem.g[r * problem.height + c]);
        gMax = Math.max(gMax, problem.g[r * problem.height + c]);
        bMin = Math.min(bMin, problem.b[r * problem.height + c]);
        bMax = Math.max(bMax, problem.b[r * problem.height + c]);
        aMin = Math.min(aMin, problem.a[r * problem.height + c]);
        aMax = Math.max(aMax, problem.a[r * problem.height + c]);
    });

    const problemSize = (rMax - rMin + 1) * (gMax - gMin + 1) * (bMax - bMin + 1) * (aMax - aMin + 1) * places.length;
    if (problemSize > 100000000) {
        // console.warn(`problem space: `, { rMin, rMax, gMin, gMax, bMin, bMax, aMin, aMax }, { places: places.length });
        return;
    }

    let curSimilarity = -1;
    let curMinColor = { r: 0, g: 0, b: 0, a: 0 };
    for (let r = rMin; r <= rMax; r++) {
        for (let g = gMin; g <= gMax; g++) {
            for (let b = bMin; b <= bMax; b++) {
                for (let a = aMin; a <= aMax; a++) {
                    const similarity = calcSimilarityOfPlaces(problem, { r, g, b, a }, places);
                    if (curSimilarity < 0 || curSimilarity > similarity) {
                        curSimilarity = similarity;
                        curMinColor = { r, g, b, a };
                    }
                }
            }
        }
    }
    return curMinColor;
};

const optimizeColor2 = (problem: Image, places: Place[], initialColor: Color) => {
    const toArray = (color: Color) => [color.r, color.g, color.b, color.a];
    const fromArray = (color: number[]) => ({ r: color[0], g: color[1], b: color[1], a: color[1] });

    let curSimilarity = calcSimilarityOfPlaces(problem, initialColor, places);
    let curMinColor = toArray(initialColor);
    while (true) {
        let finished = true;
        Array.from(Array(4)).forEach((_, channel) => {
            [-1, 1].forEach((d) => {
                while (0 <= curMinColor[channel] + d && curMinColor[channel] + d <= 255) {
                    curMinColor[channel] += d;
                    const similarity = calcSimilarityOfPlaces(problem, fromArray(curMinColor), places);
                    if (similarity < curSimilarity) {
                        finished = false;
                        curSimilarity = similarity;
                        console.log(`${curSimilarity} -> ${similarity}`);
                    } else {
                        curMinColor[channel] -= d;
                        break;
                    }
                }
            });
        });

        if (finished) {
            break;
        }
    }
    return fromArray(curMinColor);
};

const optimize = (problem: Image, moves: Move[], initialState: State): Move[] | undefined => {
    let state = initialState;

    const colorMoveId = Array.from(Array(problem.height)).map(() => Array.from(Array(problem.width)).map(() => -1));
    for (let moveId = 0; moveId < moves.length; moveId++) {
        const move = moves[moveId];
        const res = applySingleMove(move, state);
        if (res.kind == 'error') {
            console.error(`Simulation failed at ${moveId}-th move: ${res.errorMessage}`);
            return;
        }
        if (move.kind === 'color-move') {
            const block = state.blocks.get(move.blockId);
            if (!block) {
                return;
            }

            const { x1, x2, y1, y2 } = block;
            for (let r = y1; r < y2; r++) {
                for (let c = x1; c < x2; c++) {
                    colorMoveId[r][c] = moveId;
                }
            }
        }
        state = res.state;
    }

    const placesByMoveId = Array.from(Array(moves.length)).map(() => [] as { r: number; c: number }[]);
    colorMoveId.forEach((row, r) =>
        row.forEach((moveId, c) => {
            if (moveId != -1) {
                placesByMoveId[moveId].push({ r, c });
            }
        }),
    );

    const newMoves = [...moves];
    for (let moveId = 0; moveId < moves.length; moveId++) {
        const places = placesByMoveId[moveId];
        if (places.length === 0) {
            continue;
        }

        const move = newMoves[moveId];
        if (move.kind !== 'color-move') {
            continue;
        }

        const newColor = optimizeColor(problem, places);
        if (!newColor) {
            continue;
        }
        // const newColor = optimizeColor2(problem, places, move.color);
        newMoves[moveId] = { ...move, color: newColor };
    }

    return newMoves;
};

interface Place {
    r: number;
    c: number;
}
interface Color {
    r: number;
    g: number;
    b: number;
    a: number;
}
interface Solution {
    batchName: string;
}

const calcScore = (problem: Image, moves: Move[], initialState: State) => {
    let state = initialState;
    moves.forEach((move, moveId) => {
        const res = applySingleMove(move, state);
        if (res.kind == 'error') {
            console.error(`Simulation failed at ${moveId}-th move: ${res.errorMessage}`);
            return;
        }
        state = res.state;
    });

    let sum = 0.0;
    for (let r = 0; r < problem.height; r++) {
        for (let c = 0; c < problem.width; c++) {
            const stateColor = {
                r: state.r[r * problem.height + c],
                g: state.g[r * problem.height + c],
                b: state.b[r * problem.height + c],
                a: state.a[r * problem.height + c],
            };
            sum += calcSinglePixelDistance(problem, stateColor, { r, c });
        }
    }

    const score = Math.round(sum * 0.005) + state.cost;
    return score;
};

const main = async () => {
    const ranking = JSON.parse((await fsPromises.readFile('../../output/ranking.json')).toString()) as Record<string, Solution[]>;

    for (const problemId in ranking) {
        const solutions = ranking[problemId];
        const problem = await loadProblem(`../../problem/original/${problemId}.png`);
        const initial = await loadInitialBlocks(`../../problem/original/${problemId}.initial.json`);
        const initialImagePath = `../../problem/original_initial/${problemId}.initial.png`;
        const initialImage = fs.existsSync(initialImagePath) ? await loadProblem(initialImagePath) : undefined;
        const initialState = createNewState(problem.width, problem.height, initial, initialImage);

        if (solutions.length === 0) {
            continue;
        }

        const solution = solutions[0];
        const moves = await loadMoves(`../../output/${solution.batchName}/${problemId}.isl`);
        console.log(`optimizing ${solution.batchName}/${problemId} ...`);
        const newMoves = optimize(problem, moves, initialState);
        if (!newMoves) {
            console.error(`failed to optimize ${solution.batchName}/${problemId}`);
            continue;
        }

        const before = calcScore(problem, moves, initialState);
        const after = calcScore(problem, newMoves, initialState);
        if (before > after) {
            console.log(`${before} -> ${after} in ${solution.batchName}/${problemId}`);
        } else {
            console.log('no change');
        }
    }
};

const test = async () => {
    const solver = process.argv[2];
    const problemId = process.argv[3];
    const problem = await loadProblem(`../../problem/original/${problemId}.png`);
    const initial = await loadInitialBlocks(`../../problem/original/${problemId}.initial.json`);
    const initialImagePath = `../../problem/original_initial/${problemId}.initial.png`;
    const initialImage = fs.existsSync(initialImagePath) ? await loadProblem(initialImagePath) : undefined;
    const initialState = createNewState(problem.width, problem.height, initial, initialImage);
    const moves = await loadMoves(`../../output/${solver}/${problemId}.isl`);

    const score = calcScore(problem, moves, initialState);
    console.log(score);
};

main();
// test();
