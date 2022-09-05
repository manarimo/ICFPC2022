import { Image } from '../util';
import { applySingleMove, createNewState, InitialBlock, State } from '../../src/simulate';
import { Move } from '../../src/parser';

export class MergerInner {
    readonly history: State[] = [];
    readonly moveHistory: Move[] = [];
    constructor(readonly problem: Image, readonly initialState: State, readonly blockIds: string[][], readonly X: number, readonly Y: number) {
        this.history.push(initialState);
    }

    static create(problem: Image, initialBlocks: InitialBlock[] = [], initialImage?: Image) {
        let state = createNewState(problem.width, problem.height, initialBlocks, initialImage);
        const blockArea = (problem.width * problem.height) / state.blocks.size;
        const blockEdge = Math.floor(Math.sqrt(blockArea));
        const Y = problem.height / blockEdge;
        const X = problem.width / blockEdge;

        const blockIds = new Array<string[]>(X);
        for (let r = 0; r < Y; r++) {
            blockIds[r] = new Array(Y);
        }
        for (const [key, block] of state.blocks.entries()) {
            blockIds[block.x1 / blockEdge][block.y1 / blockEdge] = key;
        }

        return new MergerInner(problem, state, blockIds, X, Y);
    }

    readonly generateMoves = (): [Move[], string] => {
        const stickIds: string[] = [];
        for (let x = 0; x < this.X; x++) {
            let lineId = this.blockIds[x][0];
            for (let y = 1; y < this.Y; y++) {
                lineId = this.merge(lineId, this.blockIds[x][y]);
            }
            stickIds.push(lineId);
        }
        let finalId = stickIds[0];
        for (let x = 1; x < this.X; x++) {
            finalId = this.merge(finalId, stickIds[x]);
        }
        return [[...this.moveHistory], finalId];
    };

    readonly generateMoves2 = (): [Move[], string] => {
        // support only n * n structure
        if (this.X !== this.Y) {
            throw new Error(`not supported`);
        }

        // 10 -> 4, 16 -> 5, 20 -> 6
        const kmap = [];
        kmap[10] = 4;
        kmap[16] = 5;
        kmap[20] = 6;

        const k = kmap[this.X];
        if (k === undefined) {
            throw new Error(`not supported`);
        }

        // create stick to k
        const varStickIds: string[] = [];
        for (let x = 0; x < k; ++x) {
            let lineId = this.blockIds[x][0];
            for (let y = 1; y < this.Y; y++) {
                lineId = this.merge(lineId, this.blockIds[x][y]);
            }
            varStickIds.push(lineId);
        }

        // merge sticks
        let rectId = varStickIds[0];
        for (let x = 1; x < k; x++) {
            rectId = this.merge(rectId, varStickIds[x]);
        }

        // cut horizontally
        const horStickIds: string[] = [];
        for (let y = 1; y < this.Y; y++) {
            let horizontalId;
            [horizontalId, rectId] = this.ycut(rectId, (this.initialState.height / this.Y) * y);
            horStickIds.push(horizontalId);
        }
        horStickIds.push(rectId);

        // merge horizontal sticks
        const lineIds: string[] = [];
        for (let y = 0; y < this.Y; y++) {
            let lineId = horStickIds[y];
            for (let x = k; x < this.X; x++) {
                lineId = this.merge(lineId, this.blockIds[x][y]);
            }
            lineIds.push(lineId);
        }

        // merge all
        let finalId = lineIds[0];
        for (let y = 1; y < this.Y; y++) {
            finalId = this.merge(finalId, lineIds[y]);
        }

        return [[...this.moveHistory], finalId];
    };

    readonly ycut = (blockId: string, y: number) => {
        const tail = this.history[this.history.length - 1];
        const move: Move = {
            blockId,
            orientation: 'y',
            lineNumber: y,
            kind: 'lcut-move' as const,
        };
        const result = applySingleMove(move, tail);
        if (result.kind === 'error') {
            throw new Error(`failed to merge`);
        } else {
            this.history.push(result.state);
            this.moveHistory.push(move);
        }

        return [blockId + '.0', blockId + '.1'];
    };

    readonly merge = (blockId1: string, blockId2: string) => {
        const tail = this.history[this.history.length - 1];
        const move = {
            blockId1,
            blockId2,
            kind: 'merge-move' as const,
        };
        const result = applySingleMove(move, tail);
        if (result.kind === 'error') {
            throw new Error(`failed to merge`);
        } else {
            let newId = result.state.globalCounter.toString();
            this.history.push(result.state);
            this.moveHistory.push(move);
            return newId;
        }
    };
}
