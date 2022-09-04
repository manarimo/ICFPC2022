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
