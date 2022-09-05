import { Input, Output, Processor } from '../metaprocessor';
import { Image, runSolution } from '../util';
import { Move } from '../../src/parser';

export class ManualShifter implements Processor {
    readonly splits: [string, number, number][] = [];
    constructor(splits: string) {
        const nums = splits.split(',');
        if (nums.length % 3 != 0) {
            throw new Error('manual shifter: args must be 3x');
        }
        for (let i = 0; i < nums.length / 3; i++) {
            this.splits.push([nums[i * 3], parseInt(nums[i * 3 + 1]), parseInt(nums[i * 3 + 2])]);
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
        const moves: Move[] = [...comments, ...output.moves, ...this.shiftMoves(finalState.globalCounter, input)];
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
                    tmp = nImg.r[newPx];
                    nImg.r[newPx] = nImg.r[imgPx];
                    nImg.r[imgPx] = tmp;
                    tmp = nImg.g[newPx];
                    nImg.g[newPx] = nImg.g[imgPx];
                    nImg.g[imgPx] = tmp;
                    tmp = nImg.b[newPx];
                    nImg.b[newPx] = nImg.b[imgPx];
                    nImg.b[imgPx] = tmp;
                    tmp = nImg.a[newPx];
                    nImg.a[newPx] = nImg.a[imgPx];
                    nImg.a[imgPx] = tmp;
                    console.log(newX, newY, nImg.r[imgPx], oldX, oldY, nImg.r[newPx]);
                }
            }
            if (ori === 'x') {
                x_acc += end - begin;
            } else {
                y_acc += end - begin;
            }
            console.log(x_acc, y_acc);
        }

        return nImg;
    }

    private shiftMoves(globalCounter: number, input: Input): Move[] {
        return [];
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
