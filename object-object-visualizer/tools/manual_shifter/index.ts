import { Input, Output, Processor } from '../metaprocessor';
import { Image, runSolution } from '../util';
import { Move } from '../../src/parser';

function swap(arr: Uint8Array, i: number, j: number) {
    const tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

export class ManualShifter implements Processor {
    readonly splits: ['x' | 'y', number, number][] = [];
    readonly swaps: ['x' | 'y', number, number, number, number][] = [];
    globalCounter: number = 0;

    constructor(splits: string) {
        const nums = splits.split(',');
        if (nums.length % 3 != 0) {
            throw new Error('manual shifter: args must be 3x');
        }
        for (let i = 0; i < nums.length / 3; i++) {
            let ori = nums[i * 3];
            if (ori !== 'x' && ori !== 'y') {
                throw new Error(`manual shifter: invalid orientation ${ori}`);
            }
            this.splits.push([ori, parseInt(nums[i * 3 + 1]), parseInt(nums[i * 3 + 2])]);
        }
        this.validate();
    }

    async run(input: Input, next: (input: Input) => Promise<Output>): Promise<Output> {
        const newImage = this.shiftImage(input.image);
        const output = await next(new Input(newImage, input.initialBlocks, input.initialImage, input.palette));

        const finalState = runSolution(input.image, output.moves, input.initialBlocks, input.initialImage ?? undefined);

        const comments: Move[] = [];
        for (const split of this.splits) {
            comments.push({ kind: 'comment-move', comment: `manual_shifter: split at ${split[0]} ${split[1]} ${split[2]}` });
        }
        this.globalCounter = finalState.globalCounter;
        const moves: Move[] = [...comments, ...output.moves, ...this.shiftMoves(input)];

        /*
        console.log('P3 400 400 255');
        for (let y = 0; y < 400; y++) {
            for (let x = 0; x < 400; x++) {
                console.log(newImage.r[x + y * newImage.width], newImage.g[x + y * newImage.width], newImage.b[x + y * newImage.width]);
            }
        }
        */

        return new Output(moves);
    }

    private validate(): void {
        let x_acc = 0;
        let y_acc = 0;
        for (const split of this.splits) {
            const [ori, begin, end] = split;
            if ((ori === 'x' ? x_acc : y_acc) + (end - begin) > begin) {
                throw new Error(`manual_shifter: invalid shift ${ori} ${begin} ${end}`);
            }
            if (ori === 'x') {
                x_acc += end - begin;
            } else {
                y_acc += end - begin;
            }
            this.swaps.push([ori, ori === 'x' ? x_acc : y_acc, ori === 'x' ? x_acc + (end - begin) : y_acc + (end - begin), begin, end]);
        }
    }

    private shiftImage(image: Image): Image {
        const nImg: Image = {
            r: new Uint8Array(image.width * image.height),
            g: new Uint8Array(image.width * image.height),
            b: new Uint8Array(image.width * image.height),
            a: new Uint8Array(image.width * image.height),
            width: image.width,
            height: image.height,
        };
        for (let imgX = 0; imgX < image.width; imgX++) {
            for (let imgY = 0; imgY < image.height; ++imgY) {
                const imgPx = imgX + imgY * image.width;
                nImg.r[imgPx] = image.r[imgPx];
                nImg.g[imgPx] = image.g[imgPx];
                nImg.b[imgPx] = image.b[imgPx];
                nImg.a[imgPx] = image.a[imgPx];
            }
        }
        let x_acc = 0;
        let y_acc = 0;
        for (const split of this.splits) {
            let [ori, begin, end] = split;
            if (ori === 'y') {
                let tmp = image.height - begin;
                begin = image.height - end;
                end = tmp;
            }
            const range = end - begin;
            for (let imgX = 0; imgX < (ori === 'x' ? range : image.width); imgX++) {
                for (let imgY = 0; imgY < (ori === 'y' ? range : image.height); ++imgY) {
                    let newX = ori === 'x' ? imgX + begin : imgX;
                    let newY = ori === 'y' ? imgY + begin : imgY;
                    let oldX = ori === 'x' ? imgX + x_acc : imgX;
                    let oldY = ori === 'y' ? imgY + (image.height - y_acc - range) : imgY;
                    const newPx = newX + newY * image.width;
                    const imgPx = oldX + oldY * image.width;
                    let tmp;
                    swap(nImg.r, newPx, imgPx);
                    swap(nImg.g, newPx, imgPx);
                    swap(nImg.b, newPx, imgPx);
                    swap(nImg.a, newPx, imgPx);
                    // console.log(newX, newY, nImg.r[imgPx], oldX, oldY, nImg.r[newPx]);
                }
            }
            if (ori === 'x') {
                x_acc += end - begin;
            } else {
                y_acc += end - begin;
            }
            // console.log(x_acc, y_acc);
        }

        return nImg;
    }

    private lcutMove(blockId: string, orientation: 'x' | 'X' | 'y' | 'Y', lineNumber: number): Move {
        return { kind: 'lcut-move', blockId, orientation, lineNumber };
    }

    private mergeMove(blockId1: string, blockId2: string): Move {
        this.globalCounter++;
        return { kind: 'merge-move', blockId1, blockId2 };
    }

    private shiftMoves(input: Input): Move[] {
        let currentBlock = `${this.globalCounter}`;
        const blockIds = [];
        const moves: Move[] = [];
        const swapBlockIds = [];
        for (let i = this.swaps.length - 1; i >= 0; i--) {
            const [ori, begin1, end1, begin2, end2] = this.swaps[i];
            if (begin1 !== 0) {
                moves.push(this.lcutMove(currentBlock, ori, begin1));
                blockIds.push(`${currentBlock}.0`);
                currentBlock = `${currentBlock}.1`;
            }
            moves.push(this.lcutMove(currentBlock, ori, end1));
            blockIds.push(`${currentBlock}.0`);
            swapBlockIds.push(`${currentBlock}.0`);
            currentBlock = `${currentBlock}.1`;
            moves.push(this.lcutMove(currentBlock, ori, begin2));
            blockIds.push(`${currentBlock}.0`);
            currentBlock = `${currentBlock}.1`;
            if (end2 !== 400) {
                moves.push(this.lcutMove(currentBlock, ori, end2));
                blockIds.push(`${currentBlock}.0`);
                swapBlockIds.push(`${currentBlock}.0`);
                currentBlock = `${currentBlock}.1`;
            } else {
                swapBlockIds.push(`${currentBlock}`);
            }
            blockIds.push(currentBlock);
            moves.push({ kind: 'swap-move', blockId1: swapBlockIds[0], blockId2: swapBlockIds[1] });
            let block = blockIds[0];
            for (let j = 1; j < blockIds.length; j++) {
                let block2 = blockIds[j];
                if (block2 === swapBlockIds[0]) {
                    block2 = swapBlockIds[1];
                } else if (block2 === swapBlockIds[1]) {
                    block2 = swapBlockIds[0];
                }
                moves.push(this.mergeMove(block, block2));
                block = `${this.globalCounter}`;
            }
        }

        return moves;
        /*
        const blockIds = [];
        const moves: Move[] = [];

        const shiftPx = Math.floor(input.image.width / this.split);

        // Split the image into columns
        let currentBlock = `${globalCounter}`;
        for (let i = 0; i < this.split - 1; ++i) {
            moves.push({ kind: 'lcut-move', blockId: currentBlock, orientation: 'x', lineNumber: shiftPx * (i + 1) });
            blockIds.push(`${currentBlock}.0`);
            currentBlock = `${currentBlock}.1`;
        }
        if (shiftPx * this.split != input.image.width) {
            moves.push({ kind: 'lcut-move', blockId: currentBlock, orientation: 'x', lineNumber: shiftPx * this.split });
            blockIds.push(`${currentBlock}.0`);
        } else {
            blockIds.push(currentBlock);
        }

        // Shift columns to right (i.e. against Y-axis)
        for (let i = this.split - 1; i > 0; i--) {
            moves.push({ kind: 'swap-move', blockId1: blockIds[i], blockId2: blockIds[(i - 1 + this.split) % this.split] });
        }

        return moves;
        */
    }
}
