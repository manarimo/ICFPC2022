import {Input, Output, Processor} from '../metaprocessor';
import {Image, runSolution} from '../util';
import {Move} from '../../src/parser';

export class Shifter implements Processor {
    constructor(readonly split: number) {}

    async run(input: Input, next: (input: Input) => Promise<Output>): Promise<Output> {
        const newImage = this.shiftImage(input.image);
        const output = await next(new Input(newImage, input.initialBlocks, input.initialImage, input.palette));

        const finalState = runSolution(input.image, output.moves, input.initialBlocks, input.initialImage ?? undefined);

        const moves: Move[] = [
            { kind: 'comment-move', comment: `shifter: split in ${this.split} ways` },
            ...output.moves,
            ...this.shiftMoves(finalState.globalCounter, input)
        ];
        return new Output(moves);
    }

    private shiftImage(image: Image): Image {
        const shiftPx = Math.floor(image.width / this.split);
        const nImg: Image = {
            r: new Uint8Array(image.width * image.height),
            g: new Uint8Array(image.width * image.height),
            b: new Uint8Array(image.width * image.height),
            a: new Uint8Array(image.width * image.height),
            width: image.width,
            height: image.height,
        };

        const maxX = shiftPx * this.split;
        for (let imgX = 0; imgX < image.width; imgX++) {
            for (let imgY = 0; imgY < image.height; ++imgY) {
                const newX = (imgX - shiftPx + maxX) % maxX;
                const newPx = newX + imgY * image.width;
                const imgPx = imgX + imgY * image.width;
                nImg.r[newPx] = image.r[imgPx];
                nImg.g[newPx] = image.g[imgPx];
                nImg.b[newPx] = image.b[imgPx];
                nImg.a[newPx] = image.a[imgPx];
            }
        }

        return nImg;
    }

    private shiftMoves(globalCounter: number, input: Input): Move[] {
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
        blockIds.push(currentBlock);

        // Shift columns to right (i.e. against Y-axis)
        for (let i = this.split - 1; i > 0; i--) {
            moves.push({ kind: 'swap-move', blockId1: blockIds[i], blockId2: blockIds[(i - 1 + this.split) % this.split] });
        }

        return moves;
    }
}
