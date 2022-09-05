import { Move } from '../../src/parser';
import { applySingleMove, createNewState } from '../../src/simulate';
import { Input, Output, Processor } from '../metaprocessor';
import { calculateScore } from '../util';

const H = 400;

export class ClimbersHighPlugin implements Processor {
    constructor(readonly trials: number) {}
    async run(input: Input, next: (input: Input) => Promise<Output>): Promise<Output> {
        function evaluate(moves: Move[]): number {
            let state = createNewState(input.image.width, input.image.height, input.initialBlocks, input.initialImage ?? undefined);
            for (const move of moves) {
                const result = applySingleMove(move, state);
                if (result.kind === 'error') {
                    return 1e20;
                }
                state = result.state;
            }
            return calculateScore(input.image, state);
        }

        const output = await next(input);
        const scoreBeforeClimbing = evaluate(output.moves);

        const candidateMoveIds = [] as number[];
        for (let moveId = 0; moveId < output.moves.length; moveId++) {
            if (output.moves[moveId].kind === 'lcut-move' || output.moves[moveId].kind === 'pcut-move') {
                candidateMoveIds.push(moveId);
            }
        }

        const newMoves = [...output.moves];
        let currentScore = scoreBeforeClimbing;
        for (let trial = 0; trial < this.trials; trial++) {
            const moveId = candidateMoveIds[Math.floor(Math.random() * candidateMoveIds.length)];
            const move = newMoves[moveId];

            if (move.kind === 'lcut-move') {
                const newMove = { ...move };
                if (Math.random() > 0.5) {
                    newMove.lineNumber -= 1;
                } else {
                    newMove.lineNumber += 1;
                }
                if (newMove.lineNumber < 1 || H <= newMove.lineNumber) {
                    continue;
                }
                newMoves[moveId] = newMove;
            } else if (move.kind === 'pcut-move') {
                const newMove = { ...move };
                if (Math.random() > 0.5) {
                    if (Math.random() > 0.5) {
                        newMove.x += 1;
                    } else {
                        newMove.x -= 1;
                    }
                } else {
                    if (Math.random() > 0.5) {
                        newMove.y += 1;
                    } else {
                        newMove.y -= 1;
                    }
                }

                if (newMove.x < 1 || H <= newMove.x) {
                    continue;
                }
                if (newMove.y < 1 || H <= newMove.y) {
                    continue;
                }
                newMoves[moveId] = newMove;
            } else {
                // unreachable
                continue;
            }
            const score = evaluate(newMoves);
            if (score < currentScore) {
                // accept
                currentScore = score;
            } else {
                // reject
                newMoves[moveId] = move;
            }
        }

        return new Output([{ kind: 'comment-move', comment: `climbing: ${scoreBeforeClimbing} -> ${currentScore}` }, ...newMoves]);
    }
}
