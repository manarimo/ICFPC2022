import { ErrorResult } from "../error";
import {
  ColorMove,
  LCutMove,
  MergeMove,
  Move,
  PCutMove,
  SwapMove,
} from "../parser";

export const applySingleMove = (
  move: Move,
  state: State
): { kind: "state"; state: State } | ErrorResult => {
  switch (move.kind) {
    case "color-move": {
      return applyColorMove(move, state);
    }
    case "lcut-move": {
      return applyLCutMove(move, state);
    }
    case "pcut-move": {
      return applyPCutMove(move, state);
    }
    case "merge-move": {
      return applyMergeMove(move, state);
    }
    case "swap-move": {
      return applySwapMove(move, state);
    }
    default: {
      const nv: never = move;
      throw new Error(`unimplemented move: ${nv}`);
    }
  }
};

type Block = {
  x1: number;
  y1: number;
  x2: number;
  y2: number;
};

const undefinedBlockError = (blockId: string) => {
  return {
    kind: "error" as const,
    errorMessage: `undefined block: ${blockId}`,
  };
};

const applyColorMove = (move: ColorMove, state: State) => {
  const block = state.blocks.get(move.blockId);
  if (!block) {
    return undefinedBlockError(move.blockId);
  }

  const nextState = copyRGBA(state);
  for (let x = block.x1; x < block.x2; x++) {
    for (let y = block.y1; y < block.y2; y++) {
      setColorInPlace(nextState, x, y, move.color);
    }
  }

  return {
    kind: "state" as const,
    state: nextState,
  };
};

const applyLCutMove = (move: LCutMove, state: State) => {
  const block = state.blocks.get(move.blockId);
  if (!block) {
    return undefinedBlockError(move.blockId);
  }

  const { lineNumber, blockId, orientation } = move;
  const { x1, x2, y1, y2 } = block;
  if (orientation === "x" || orientation === "X") {
    const x = lineNumber;
    const left = { x1, y1, x2: x, y2 };
    const right = { x1: x, y1, x2, y2 };
    const nextState = copyBlocks(state);
    nextState.blocks.delete(blockId);
    nextState.blocks.set(`${blockId}.0`, left);
    nextState.blocks.set(`${blockId}.1`, right);
    return { kind: "state" as const, state: nextState };
  } else {
    const y = lineNumber;
    const bottom = {
      x1,
      y1,
      x2,
      y2: y,
    };
    const top = {
      x1,
      y1: y,
      x2,
      y2,
    };
    const nextState = copyBlocks(state);
    nextState.blocks.delete(blockId);
    nextState.blocks.set(`${blockId}.0`, bottom);
    nextState.blocks.set(`${blockId}.1`, top);
    return { kind: "state" as const, state: nextState };
  }
};

const applyPCutMove = (move: PCutMove, state: State) => {
  const block = state.blocks.get(move.blockId);
  if (!block) {
    return undefinedBlockError(move.blockId);
  }

  const { x1, x2, y1, y2 } = block;
  const { x, y, blockId } = move;
  if (x < x1 || x2 < x || y < y1 || y2 < y) {
    return {
      kind: "error" as const,
      errorMessage: `pcut-move-error: point (${x},${y}) is outside of block ${block}`,
    };
  }

  const leftBottom = {
    x1,
    y1,
    x2: x,
    y2: y,
  };
  const rightBottom = {
    x1: x,
    y1,
    x2,
    y2: y,
  };
  const rightTop = {
    x1: x,
    y1: y,
    x2,
    y2,
  };
  const leftTop = {
    x1,
    y1: y,
    x2: x,
    y2,
  };

  const nextState = copyBlocks(state);
  nextState.blocks.delete(blockId);

  nextState.blocks.set(`${blockId}.0`, { ...leftBottom });
  nextState.blocks.set(`${blockId}.1`, { ...rightBottom });
  nextState.blocks.set(`${blockId}.2`, { ...rightTop });
  nextState.blocks.set(`${blockId}.3`, { ...leftTop });
  return { kind: "state" as const, state: nextState };
};

const applySwapMove = (move: SwapMove, state: State) => {
  const block1 = state.blocks.get(move.blockId1);
  if (!block1) {
    return undefinedBlockError(move.blockId1);
  }
  const block2 = state.blocks.get(move.blockId2);
  if (!block2) {
    return undefinedBlockError(move.blockId2);
  }

  const dx1 = block1.x2 - block1.x1;
  const dy1 = block1.y2 - block1.y1;

  const dx2 = block2.x2 - block2.x1;
  const dy2 = block2.y2 - block2.y1;

  if (dx1 !== dx2 || dy1 !== dy2) {
    return {
      kind: "error" as const,
      errorMessage: `failed to swap ${block1} and ${block2}`,
    };
  }

  const dx = dx1;
  const dy = dy1;
  const nextState = copyRGBA(state);
  for (let px = 0; px < dx; px++) {
    for (let py = 0; py < dy; py++) {
      setColorInPlace(
        nextState,
        block1.x1 + px,
        block1.y1 + py,
        getColor(state, block2.x1 + px, block2.y1 + py)
      );
      setColorInPlace(
        nextState,
        block2.x1 + px,
        block2.y1 + py,
        getColor(state, block1.x1 + px, block1.y1 + py)
      );
    }
  }

  return {
    kind: "state" as const,
    state: nextState,
  };
};

const applyMergeMove = (move: MergeMove, state: State) => {
  const block1 = state.blocks.get(move.blockId1);
  if (!block1) {
    return undefinedBlockError(move.blockId1);
  }
  const block2 = state.blocks.get(move.blockId2);
  if (!block2) {
    return undefinedBlockError(move.blockId2);
  }

  // TODO rect check
  if (
    block1.x1 === block2.x1 &&
    block1.x2 === block2.x2 &&
    (block1.y2 === block2.y1 || block1.y1 === block2.y2)
  ) {
    // top-bottom
  } else if (
    block1.y1 === block2.y1 &&
    block1.y2 === block2.y2 &&
    (block1.x2 === block2.x1 || block1.x1 === block2.x2)
  ) {
    // left-right
  } else {
    return {
      kind: "error" as const,
      errorMessage: `merged convex ${block1} + ${block2} is not rectangle`,
    };
  }

  const x1 = Math.min(block1.x1, block2.x1);
  const x2 = Math.max(block1.x2, block2.x2);
  const y1 = Math.min(block1.y1, block2.y1);
  const y2 = Math.max(block1.y2, block2.y2);

  const majorSet = new Set<string>();
  Array.from(state.blocks.keys()).forEach((key) => {
    majorSet.add(key.split(".")[0]);
  });

  let index = 0;
  while (majorSet.has(index.toString())) {
    index += 1;
  }

  const nextState = copyBlocks(state);
  nextState.blocks.delete(move.blockId1);
  nextState.blocks.delete(move.blockId2);
  nextState.blocks.set(index.toString(), { x1, y1, x2, y2 });
  return { kind: "state" as const, state: nextState };
};

type State = {
  blocks: Map<string, Block>;
  width: number;
  height: number;

  r: Uint8Array;
  g: Uint8Array;
  b: Uint8Array;
  a: Uint8Array;
};

export const createNewState = (width: number, height: number): State => {
  const state = {
    blocks: new Map([
      [
        "0",
        {
          x1: 0,
          y1: 0,
          x2: width,
          y2: height,
        },
      ],
    ]),
    width,
    height,
    r: new Uint8Array(width * height).fill(0),
    g: new Uint8Array(width * height).fill(0),
    b: new Uint8Array(width * height).fill(0),
    a: new Uint8Array(width * height).fill(0),
  };
  return state;
};

const copyRGBA = (state: State): State => {
  return {
    ...state,
    r: new Uint8Array(state.r),
    g: new Uint8Array(state.g),
    b: new Uint8Array(state.b),
    a: new Uint8Array(state.a),
  };
};

const copyBlocks = (state: State): State => {
  return { ...state, blocks: new Map(state.blocks) };
};

export const getColor = (state: State, x: number, y: number) => {
  return {
    r: state.r[y * state.width + x],
    g: state.g[y * state.width + x],
    b: state.b[y * state.width + x],
    a: state.a[y * state.width + x],
  };
};

const setColorInPlace = (
  state: State,
  x: number,
  y: number,
  color: { r: number; g: number; b: number; a: number }
) => {
  state.r[y * state.width + x] = color.r;
  state.g[y * state.width + x] = color.g;
  state.b[y * state.width + x] = color.b;
  state.a[y * state.width + x] = color.a;
};
