import { MergerInner } from './merger';
import { Move } from '../../src/parser';
import { Input, Output, Processor } from '../metaprocessor';

export class Merger implements Processor {
    readonly filterOut0255 = (moves: Move[]) => {
        const newMoves: Move[] = [];
        for (let i = 0; i < moves.length; i++) {
            const currentMove = moves[i];
            if (currentMove.kind === 'color-move') {
                if (
                    currentMove.blockId === '0' &&
                    currentMove.color.r === 0 &&
                    currentMove.color.g === 0 &&
                    currentMove.color.b === 0 &&
                    currentMove.color.a === 0
                ) {
                    continue;
                }
            }
            newMoves.push(currentMove);
        }
        return newMoves;
    };

    readonly filterOutDuplicatedColorForSameBlock = (moves: Move[]) => {
        const newMoves: Move[] = [];
        L: for (let i = 0; i < moves.length; i++) {
            const currentMove = moves[i];
            if (currentMove.kind === 'color-move') {
                for (let j = i + 1; j < moves.length; j++) {
                    const futureMove = moves[j];
                    if (futureMove.kind === 'color-move' && futureMove.blockId === currentMove.blockId) {
                        continue L;
                    }
                }
            }
            newMoves.push(currentMove);
        }
        return newMoves;
    };

    readonly run = async (input: Input, next: (input: Input) => Promise<Output>) => {
        if (input.initialBlocks.length == 1) {
            // The "initial block" must be a whole canvas. Just skip doing anything special.
            return next(input);
        }

        const merger = MergerInner.create(input.image, input.initialBlocks, input.initialImage ?? undefined);
        const [moves, finalId] = merger.generateMoves2();
        moves.push({
            color: { r: 255, g: 255, b: 255, a: 255 },
            blockId: finalId,
            kind: 'color-move' as const,
        });
        const output = await next({
            image: input.image,
            initialBlocks: [
                {
                    blockId: '0',
                    bottomLeft: [0, 0],
                    topRight: [400, 400],
                    color: [255, 255, 255, 255],
                },
            ],
            initialImage: input.initialImage,
            palette: input.palette,
        });

        const overrideId = (id: string) => {
            const ids = id.split('.');
            ids[0] = (parseInt(ids[0]) + parseInt(finalId)).toString();
            return ids.join('.');
        };

        const overrideMove = (move: Move) => {
            switch (move.kind) {
                case 'comment-move':
                    return move;
                case 'pcut-move': {
                    return {
                        ...move,
                        blockId: overrideId(move.blockId),
                    };
                }
                case 'lcut-move': {
                    return {
                        ...move,
                        blockId: overrideId(move.blockId),
                    };
                }
                case 'color-move': {
                    return {
                        ...move,
                        blockId: overrideId(move.blockId),
                    };
                }
                case 'swap-move': {
                    return {
                        ...move,
                        blockId1: overrideId(move.blockId1),
                        blockId2: overrideId(move.blockId2),
                    };
                }
                case 'merge-move': {
                    return {
                        ...move,
                        blockId1: overrideId(move.blockId1),
                        blockId2: overrideId(move.blockId2),
                    };
                }
            }
        };

        const no0255 = this.filterOut0255(output.moves);
        const overriddenMoves = [...moves, ...no0255.map(overrideMove)];
        return new Output(this.filterOutDuplicatedColorForSameBlock(overriddenMoves));
    };
}
