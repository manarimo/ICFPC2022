import { parseProgram } from '../../src/parser';
import * as fsPromises from 'fs/promises';
import { loadInitialBlocks, loadMoves, loadProblem } from '../util';
import { applySingleMove, createNewState } from '../../src/simulate';

interface Solution {
    batchName: string;
}

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

        let state = initialState;
        for (let i = 0; i < moves.length; i++) {
            const move = moves[i];
            const res = applySingleMove(move, state);
            if (res.kind == 'error') {
                console.error(`Simulation failed at ${i}-th move: ${res.errorMessage}`);
                break;
            }
            state = res.state;
        }
    }
};

main();
