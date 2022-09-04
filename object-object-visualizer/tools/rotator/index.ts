import { calculateScore, Image, runSolution } from '../util';
import { InitialBlock } from '../../src/simulate';
import { Move } from '../../src/parser';
import { Input, Output, Processor } from '../metaprocessor';

export interface RotationSpec {
    rotate: number;
    flip: boolean;
}

export class Rotator implements Processor {
    constructor(readonly rotations: RotationSpec[]) {}

    readonly rotate90 = (image: Image): Image => {
        const nImg: Image = {
            r: new Uint8Array(image.width * image.height),
            g: new Uint8Array(image.width * image.height),
            b: new Uint8Array(image.width * image.height),
            a: new Uint8Array(image.width * image.height),
            width: image.height,
            height: image.width,
        };
        for (let x = 0; x < image.height; x++) {
            for (let y = 0; y < image.width; y++) {
                let cx = image.height - y - 1;
                let cy = x;

                const newIdx = x + y * image.height;
                const originalIdx = cx + cy * image.height;
                nImg.r[originalIdx] = image.r[newIdx];
                nImg.g[originalIdx] = image.g[newIdx];
                nImg.b[originalIdx] = image.b[newIdx];
                nImg.a[originalIdx] = image.a[newIdx];
                // なんかここ逆にしたら動いた
            }
        }
        return nImg;
    };

    readonly flipImage = (image: Image): Image => {
        const nImg: Image = {
            r: new Uint8Array(image.width * image.height),
            g: new Uint8Array(image.width * image.height),
            b: new Uint8Array(image.width * image.height),
            a: new Uint8Array(image.width * image.height),
            width: image.width,
            height: image.height,
        };
        for (let x = 0; x < image.width; x++) {
            for (let y = 0; y < image.height; y++) {
                let cx = image.width - x - 1;
                let cy = y;

                const newIdx = x + y * image.width;
                const originalIdx = cx + cy * image.width;
                nImg.r[newIdx] = image.r[originalIdx];
                nImg.g[newIdx] = image.g[originalIdx];
                nImg.b[newIdx] = image.b[originalIdx];
                nImg.a[newIdx] = image.a[originalIdx];
            }
        }
        return nImg;
    };

    readonly flipMoves = (moves: Move[], initialBlocks: InitialBlock[], image: Image) => {
        const block_ids: { [key: string]: string } = {};
        block_ids[initialBlocks[0].blockId] = initialBlocks[0].blockId;
        let global_counter = initialBlocks.length;

        const flipMove = (move: Move): Move => {
            switch (move.kind) {
                case 'lcut-move': {
                    const original_id = move.blockId;
                    const mapped_id = block_ids[original_id];
                    const isX = move.orientation == 'x' || move.orientation == 'X';
                    for (let sub_block = 0; sub_block < 2; sub_block++) {
                        if (isX) {
                            block_ids[original_id + `.${sub_block}`] = mapped_id + `.${sub_block ^ 1}`;
                        } else {
                            block_ids[original_id + `.${sub_block}`] = mapped_id + `.${sub_block}`;
                        }
                    }
                    return {
                        kind: 'lcut-move',
                        blockId: mapped_id,
                        orientation: move.orientation,
                        lineNumber: isX ? image.width - move.lineNumber : move.lineNumber,
                    };
                }
                case 'pcut-move': {
                    const original_id = move.blockId;
                    const mapped_id = block_ids[original_id];
                    for (let sub_block = 0; sub_block < 4; sub_block++) {
                        block_ids[original_id + `.${sub_block}`] = mapped_id + `.${sub_block ^ 1}`;
                    }
                    const { x: px, y: py } = move;
                    return {
                        blockId: mapped_id,
                        x: image.width - px,
                        y: py,
                        kind: 'pcut-move',
                    };
                }
                case 'merge-move': {
                    const original_id1 = move.blockId1;
                    const original_id2 = move.blockId2;

                    const mapped_id1 = block_ids[original_id1];
                    const mapped_id2 = block_ids[original_id2];

                    const new_block_id = global_counter.toString();
                    global_counter += 1;
                    block_ids[new_block_id] = new_block_id;

                    return {
                        blockId1: mapped_id1,
                        blockId2: mapped_id2,
                        kind: 'merge-move' as const,
                    };
                }
                case 'color-move': {
                    return {
                        kind: 'color-move',
                        blockId: block_ids[move.blockId],
                        color: move.color,
                    };
                }
                case 'swap-move': {
                    return {
                        kind: 'swap-move',
                        blockId1: block_ids[move.blockId1],
                        blockId2: block_ids[move.blockId2],
                    };
                }
                case 'comment-move': {
                    return move;
                }
            }
        };

        return moves.map(flipMove);
    };

    readonly rotateMoves = (moves: Move[], initialBlocks: InitialBlock[], image: Image) => {
        const block_ids: { [key: string]: string } = {};
        block_ids[initialBlocks[0].blockId] = initialBlocks[0].blockId;
        let global_counter = initialBlocks.length;

        const rotateMove = (move: Move): Move => {
            switch (move.kind) {
                case 'lcut-move': {
                    const original_id = move.blockId;
                    const mapped_id = block_ids[original_id];
                    const isX = move.orientation == 'x' || move.orientation == 'X';
                    for (let sub_block = 0; sub_block < 2; sub_block++) {
                        if (!isX) {
                            block_ids[original_id + `.${sub_block}`] = mapped_id + `.${sub_block ^ 1}`;
                        } else {
                            block_ids[original_id + `.${sub_block}`] = mapped_id + `.${sub_block}`;
                        }
                    }
                    return {
                        kind: 'lcut-move',
                        blockId: mapped_id,
                        orientation: isX ? 'y' : 'x',
                        lineNumber: isX ? move.lineNumber : image.height - move.lineNumber,
                    };
                }
                case 'pcut-move': {
                    const original_id = move.blockId;
                    const mapped_id = block_ids[original_id];
                    for (let sub_block = 0; sub_block < 4; sub_block++) {
                        block_ids[original_id + `.${sub_block}`] = mapped_id + `.${(sub_block + 1) % 4}`;
                    }
                    const { x: px, y: py } = move;
                    return {
                        blockId: mapped_id,
                        x: image.height - py,
                        y: px,
                        kind: 'pcut-move',
                    };
                }
                case 'merge-move': {
                    const original_id1 = move.blockId1;
                    const original_id2 = move.blockId2;

                    const mapped_id1 = block_ids[original_id1];
                    const mapped_id2 = block_ids[original_id2];

                    const new_block_id = global_counter.toString();
                    global_counter += 1;
                    block_ids[new_block_id] = new_block_id;

                    return {
                        blockId1: mapped_id1,
                        blockId2: mapped_id2,
                        kind: 'merge-move' as const,
                    };
                }
                case 'color-move': {
                    return {
                        kind: 'color-move',
                        blockId: block_ids[move.blockId],
                        color: move.color,
                    };
                }
                case 'swap-move': {
                    return {
                        kind: 'swap-move',
                        blockId1: block_ids[move.blockId1],
                        blockId2: block_ids[move.blockId2],
                    };
                }
                case 'comment-move': {
                    return move;
                }
            }
        };

        return moves.map(rotateMove);
    };

    readonly run = async (input: Input, next: (input: Input) => Promise<Output>) => {
        let bestMoves: Move[] = [];
        let bestScore = 1e9;
        let bestRotation: RotationSpec | undefined = undefined;

        for (let spec of this.rotations) {
            console.log(`rotator: running ${JSON.stringify(spec)}`);

            const output = await this.runInternal(input, next, spec.rotate, spec.flip);
            const state = await runSolution(input.image, output.moves, input.initialBlocks);
            const score = calculateScore(input.image, state);
            if (bestScore > score) {
                bestScore = score;
                bestMoves = output.moves;
                bestRotation = spec;
            }
        }

        bestMoves.unshift({ kind: 'comment-move', comment: `rotator: ${JSON.stringify(bestRotation)}` });
        return new Output(bestMoves);
    };

    readonly runInternal = async (input: Input, next: (input: Input) => Promise<Output>, rotate: number, flip: boolean) => {
        if (input.initialBlocks.length !== 1) {
            throw new Error(`not supported`);
        }

        let nextImage: Image = input.image;
        if (flip) {
            nextImage = this.flipImage(nextImage);
        }
        for (let i = 0; i < rotate % 4; i++) {
            nextImage = this.rotate90(nextImage);
        }

        const output = await next(new Input(nextImage, input.initialBlocks));

        let newMoves = [...output.moves];

        for (let i = 0; i < rotate % 4; i++) {
            newMoves = this.rotateMoves(newMoves, input.initialBlocks, input.image);
        }
        if (flip) {
            newMoves = this.flipMoves(newMoves, input.initialBlocks, input.image);
        }

        return new Output(newMoves);
    };
}
