import { Move, parseProgram } from '../../src/parser';
import * as fsPromises from 'fs/promises';
import { Image, loadInitialBlocks, loadMoves, loadProblem } from '../util';
import { applySingleMove, calculatePixelSimilarity, createNewState, State } from '../../src/simulate';

const calcSinglePixelDistance = (problem: Image, color: Color, { r, c }: Place): number => {
    const px = (problem.height - r - 1) * problem.width + c;

    const rDiff = Math.pow(problem.r[px] - color.r, 2);
    const gDiff = Math.pow(problem.g[px] - color.g, 2);
    const bDiff = Math.pow(problem.b[px] - color.b, 2);
    const aDiff = Math.pow(problem.a[px] - color.a, 2);
    return Math.sqrt(rDiff + gDiff + bDiff + aDiff);
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
        const initialState = createNewState(problem.width, problem.height, initial);

        if (solutions.length === 0) {
            continue;
        }

        const solution = solutions[0];
        const moves = await loadMoves(`../../output/${solution.batchName}/${problemId}.isl`);
        const newMoves = optimize(problem, moves, initialState);
        if (!newMoves) {
            continue;
        }

        const before = calcScore(problem, moves, initialState);
        const after = calcScore(problem, newMoves, initialState);
        if (before > after) {
            console.log(`${before} -> ${after} in ${solution.batchName}/${problemId}`);
        }
    }
};

const test = async () => {
    const solver = process.argv[2];
    const problemId = process.argv[3];
    const problem = await loadProblem(`../../problem/original/${problemId}.png`);
    const initial = await loadInitialBlocks(`../../problem/original/${problemId}.initial.json`);
    const initialState = createNewState(problem.width, problem.height, initial);
    const moves = await loadMoves(`../../output/${solver}/${problemId}.isl`);

    const score = calcScore(problem, moves, initialState);
    console.log(score);
};

main();
// test();
