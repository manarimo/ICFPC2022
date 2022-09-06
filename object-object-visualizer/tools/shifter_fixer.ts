import * as fsPromises from 'fs/promises';
import { allSolutions, loadMoves, moveToString } from './util';
import { SwapMove } from '../src/parser';

async function main() {
    for await (const solution of allSolutions()) {
        if (!solution.batchName.match(/.*shift-test.*/)) {
            continue;
        }

        const path = solution.solutionPath();
        console.log(path);

        const moves = await loadMoves(path);
        let idx = moves.length - 1;
        while (moves[idx].kind == 'swap-move') {
            idx--;
        }
        idx++;
        if (moves[idx].kind != 'swap-move') {
            throw new Error('Move must be swap');
        }
        const move = moves[idx] as SwapMove;
        const baseId = move.blockId1;
        for (let i = idx; i < moves.length; ++i) {
            (moves[i] as SwapMove).blockId1 = baseId;
        }

        await fsPromises.writeFile(path, moves.map(moveToString).join('\n'));
    }
}

main();
