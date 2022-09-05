import { Input, Output, Processor } from '../metaprocessor';
import { ColorMove, Move } from '../../src/parser';
import { applySingleMove, createNewState } from '../../src/simulate';
import { calculateScore } from '../util';


export class FreelunchPlugin implements Processor {
    async run(input: Input, next: (input: Input) => Promise<Output>): Promise<Output> {
        const output = await next(input);
        const lastTouchIndexes = new Array(input.image.width);
        for (let x = 0; x < input.image.width; x++) {
            lastTouchIndexes[x] = new Array(input.image.height).fill(-1);
        }

        let state = createNewState(input.image.width, input.image.height, input.initialBlocks, input.initialImage ?? undefined);
        for (let moveIndex = 0; moveIndex < output.moves.length; moveIndex++) {
            const move = output.moves[moveIndex];

            if (move.kind === "color-move") {
                const block = state.blocks.get(move.blockId);
                if (!block) {
                    throw Error(`no block id ${move.blockId} found`);
                }
                for (let x = block.x1; x < block.x2; x++) {
                    for (let y = block.y1; y < block.y2; y++) {
                        lastTouchIndexes[x][y] = moveIndex;
                    }
                }
            }
            const result = applySingleMove(move, state);
            if (result.kind === "error") {
                throw Error("failed to apply state: " + result.errorMessage); 
            }
            state = result.state;
        }

        const coloringPoints: Map<number, {x: number, y: number}[]> = new Map(); 
        for (let x = 0; x < input.image.width; x++) {
            for (let y = 0; y < input.image.height; y++) {
                const moveIndex = lastTouchIndexes[x][y];
                if (!coloringPoints.has(moveIndex)) {
                    coloringPoints.set(moveIndex, []);
                }
                coloringPoints.get(moveIndex)?.push({x, y});
            }
        }

        function evaluate(moves: Move[]): number {
            let state = createNewState(input.image.width, input.image.height, input.initialBlocks, input.initialImage ?? undefined);
            for (const move of moves) {
                const result = applySingleMove(move, state);
                if (result.kind === "error") {
                    throw new Error("failed to apply state: " + result.errorMessage);
                }
                state = result.state;
            }
            return calculateScore(input.image, state);
        };
        const costBeforeOptimize = evaluate(output.moves);

        let moves = [...output.moves];
        for (const [moveIndex, points] of coloringPoints.entries()) {
            const move = output.moves[moveIndex] as ColorMove;
            const initialColorVector = [move.color.r, move.color.g, move.color.b, move.color.a];
            const colorVectors = points
                .map(point => (input.image.height - 1 - point.y) * input.image.width + point.x)
                .map(px => [input.image.r[px], input.image.g[px], input.image.b[px], input.image.a[px]]);
            const optimalColorVector = geometricMedian(colorVectors).map(round);

            if (distanceSum(colorVectors, initialColorVector) > distanceSum(colorVectors, optimalColorVector)) {
                moves[moveIndex] = {
                    ...move,
                    color: {r: optimalColorVector[0], g: optimalColorVector[1], b: optimalColorVector[2], a: optimalColorVector[3]},
                }
            }
        }

        const costAfterOptimize = evaluate(moves);
        return new Output([{kind: "comment-move", comment: `freelunch: ${costBeforeOptimize} -> ${costAfterOptimize}`}, ...moves]);
    }
}

function geometricMedian(points: number[][]): number[] {
    let v = [0., 0., 0., 0.];
    for (let _ = 0; _ < 20; _++) {
        let nv = [0., 0., 0., 0.];
        let coef = 0.;
        for (const point of points) {
            let dist = distance(point, v); 
            if (dist <= 1e-12) {
                return v;
            }
            coef += 1. / dist;
        }
        for (const point of points) {
            let dist = distance(point, v);
            for (let d = 0; d < 4; d++) {
                nv[d] += point[d] / dist / coef;
            }
        }
        v = nv;
    }
    return v;
}

function distanceSum(points: number[][], origin: number[]): number {
    let sum = 0;
    for (const point of points) {
        sum += distance(point, origin);
    }
    return sum;
}

function distance(v1: number[], v2: number[]): number {
    let norm = 0.;
    for (let i = 0; i < v1.length; i++) {
        norm += Math.pow(v1[i] - v2[i], 2);
    }
    return Math.pow(norm, 0.5);
}

function round(x: number): number {
    if (x <= 0) {
        return 0;
    } else if (x >= 255) {
        return 255;
    } else {
        return Math.round(x);
    }
}