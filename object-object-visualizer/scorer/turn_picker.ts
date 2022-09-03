import * as fsPromises from 'fs/promises';
import {calculateScore, Image, loadMoves, loadProblem, Solution, topNSolutions} from "./util";
import {applySingleMove, createNewState} from "../src/simulate";
import {Move} from "../src/parser";

async function main() {
    const problemCache: Record<string, Image> = {};
    const bestTurns: Record<string, {score: number, turn: number, spec: Solution, moves: Move[]}> = {};

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

        let bestTurn: number = 0;
        let bestScore = 1e9;
        let state = createNewState(problem.width, problem.height);
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
        if (bestTurns[solution.problemId] === undefined || bestTurns[solution.problemId].score > bestScore) {
            bestTurns[solution.problemId] = {turn: bestTurn, score: bestScore, spec: solution, moves};
        }
    }

    const batchName = process.argv[2];
    const outDir = `../../output/${batchName}`;
    await fsPromises.mkdir(outDir, {recursive: true});

    for (let best of Object.values(bestTurns)) {
        console.log(`Best move of ${best.spec.problemId}: ${best.spec.batchName} at ${best.turn}`);

        const bestMoves = best.moves.slice(0, best.turn + 1);
        const code = bestMoves.map((move) => {
            switch (move.kind) {
                case 'lcut-move':
                    return `cut [${move.blockId}] [${move.orientation}] [${move.lineNumber}]`;
                case 'pcut-move':
                    return `cut [${move.blockId}] [${move.x}, ${move.y}]`;
                case 'color-move':
                    return `color [${move.blockId}] [${move.color.r}, ${move.color.g}, ${move.color.b}, ${move.color.a}]`;
                case 'swap-move':
                    return `swap [${move.blockId1}] [${move.blockId2}]`;
                case 'merge-move':
                    return `merge [${move.blockId1}] [${move.blockId2}]`;
            }
        });
        await fsPromises.writeFile(`${outDir}/${best.spec.problemId}.isl`, code.join("\n"));
    }
}

main();