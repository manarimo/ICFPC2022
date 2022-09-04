import { MergerInner } from './merger';
import { Move } from '../../src/parser';
import { Input, Output, Processor } from '../metaprocessor';

export class Merger implements Processor {
    readonly run = async (input: Input, next: (input: Input) => Promise<Output>) => {
        const merger = MergerInner.create(input.image, input.initialBlocks, input.initialImage ?? undefined);
        const [moves, finalId] = merger.generateMoves();
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

        const overriddenMoves = [...moves, ...output.moves.map(overrideMove)];
        return new Output(overriddenMoves);
    };
}
