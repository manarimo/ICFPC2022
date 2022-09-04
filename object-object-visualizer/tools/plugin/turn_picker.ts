import {Input, Output, Processor} from "../metaprocessor";
import {applySingleMove, createNewState} from "../../src/simulate";
import {calculateScore} from "../util";

export class TurnPicker implements Processor {
    readonly run = async (input: Input, next: (input: Input) => Promise<Output>) => {
        const output = await next(input);
        let bestTurn: number = 0;
        let bestScore = 1e9;
        let state = createNewState(input.image.width, input.image.height, input.initialBlocks);
        output.moves.forEach((move, i) => {
            const res = applySingleMove(move, state);
            if (res.kind == 'error') {
                throw new Error(`Simulation failed at ${i}-th move: ${res.errorMessage}`);
            }
            state = res.state;
            const newScore = calculateScore(input.image, state);
            if (newScore < bestScore) {
                bestTurn = i;
                bestScore = newScore;
            }
        });
        const finalMoves = output.moves.slice(0, bestTurn);
        return new Output(finalMoves);
    }
}
