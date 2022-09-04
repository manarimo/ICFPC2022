import * as fsPromises from 'fs/promises';
import * as fs from 'fs';
import { calculateScore, Image, loadInitialBlocks, loadMoves, loadProblem, moveToString, Solution, topNSolutions } from './util';
import { applySingleMove, createNewState, InitialBlock } from '../src/simulate';
import { Move } from '../src/parser';

async function main() {
    const problemCache: Record<string, Image> = {};
    const initialBlockCache: Record<string, InitialBlock[] | undefined> = {};
    const bestTurns: Record<string, { score: number; turn: number; spec: Solution; moves: Move[] }> = {};

    for await (let solution of topNSolutions(3)) {
        console.log(`Scanning ${solution.batchName}/${solution.problemId}...`);

        let problem: Image;
        if (solution.problemId in problemCache) {
            problem = problemCache[solution.problemId];
        } else {
            const problemPath = `../../problem/original/${solution.problemId}.png`;
            problem = problemCache[solution.problemId] = await loadProblem(problemPath);
        }

        const solutionPath = `../../output/${solution.batchName}/${solution.problemId}.isl`;
        const moves = await loadMoves(solutionPath);

        let initialBlocks: InitialBlock[] | undefined;
        if (solution.problemId in initialBlockCache) {
            initialBlocks = initialBlockCache[solution.problemId];
        } else {
            const initialBlocksPath = `../../problem/original/${solution.problemId}.initial.json`;
            initialBlocks = initialBlockCache[solution.problemId] = (await loadInitialBlocks(initialBlocksPath)) ?? undefined;
        }

        const initialImagePath = `../../problem/original_initial/${solution.problemId}.initial.png`;
        const initialImage = fs.existsSync(initialImagePath) ? await loadProblem(initialImagePath) : undefined;

        let bestTurn: number = 0;
        let bestScore = 1e9;
        let state = createNewState(problem.width, problem.height, initialBlocks, initialImage);
        moves.forEach((move, i) => {
            const res = applySingleMove(move, state);
            if (res.kind == 'error') {
                throw new Error(`Simulation failed at ${i}-th move: ${res.errorMessage}`);
            }
            state = res.state;
            const newScore = calculateScore(problem, state);
            if (newScore < bestScore) {
                bestTurn = i;
                bestScore = newScore;
            }
        });

        if (bestTurn < moves.length - 1) {
            if (bestTurns[solution.problemId] === undefined || bestTurns[solution.problemId].score > bestScore) {
                console.log(`Best for ${solution.problemId}: truncate ${solution.batchName} at ${bestTurn + 1} (/ ${moves.length})`);
                bestTurns[solution.problemId] = { turn: bestTurn, score: bestScore, spec: solution, moves };
            }
        }
    }

    const batchName = process.argv[2];
    const outDir = `../../output/${batchName}`;
    await fsPromises.mkdir(outDir, { recursive: true });

    for (let best of Object.values(bestTurns)) {
        console.log(`Best move of ${best.spec.problemId}: ${best.spec.batchName} at ${best.turn}`);

        const bestMoves = best.moves.slice(0, best.turn + 1);
        const code = bestMoves.map(moveToString);
        code.unshift(`# Based on ${best.spec.batchName}`);
        await fsPromises.writeFile(`${outDir}/${best.spec.problemId}.isl`, code.join('\n'));
    }
}

main();
