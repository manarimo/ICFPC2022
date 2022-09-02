import { ErrorResult } from "../error";
import { ColorMove, LCutMove, Move, PCutMove, SwapMove } from "../parser";

export const applySingleMove = (
  move: Move,
  state: Map<string, Block>
): { kind: "state"; state: Map<string, Block> } | ErrorResult => {
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
      return {
        kind: "error",
        errorMessage: "sorry, merge-move is not supported yet",
      };
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

export type Block = {
  color: {
    r: number;
    g: number;
    b: number;
    a: number;
  };
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

const applyColorMove = (move: ColorMove, state: Map<string, Block>) => {
  const block = state.get(move.blockId);
  if (!block) {
    return undefinedBlockError(move.blockId);
  }

  const newBlock = { ...block };
  newBlock.color = move.color;
  const nextState = new Map(state);
  nextState.set(move.blockId, newBlock);

  return {
    kind: "state" as const,
    state: nextState,
  };
};

const applyLCutMove = (move: LCutMove, state: Map<string, Block>) => {
  const block = state.get(move.blockId);
  if (!block) {
    return undefinedBlockError(move.blockId);
  }

  const { lineNumber, blockId, orientation } = move;
  const { x1, x2, y1, y2, color } = block;
  if (orientation === "x" || orientation === "X") {
    const x = lineNumber;
    const left = { x1, y1, x2: x, y2 };
    const right = { x1: x, y1, x2, y2 };
    const nextState = new Map(state);
    nextState.delete(blockId);
    nextState.set(`${blockId}.0`, { ...left, color });
    nextState.set(`${blockId}.1`, { ...right, color });
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
    const nextState = new Map(state);
    nextState.delete(blockId);
    nextState.set(`${blockId}.0`, { ...bottom, color });
    nextState.set(`${blockId}.1`, { ...top, color });
    return { kind: "state" as const, state: nextState };
  }
};

const applyPCutMove = (move: PCutMove, state: Map<string, Block>) => {
  const block = state.get(move.blockId);
  if (!block) {
    return undefinedBlockError(move.blockId);
  }

  const { x1, x2, y1, y2, color } = block;
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

  const nextState = new Map(state);
  nextState.delete(blockId);

  nextState.set(`${blockId}.0`, { ...leftBottom, color });
  nextState.set(`${blockId}.1`, { ...rightBottom, color });
  nextState.set(`${blockId}.2`, { ...rightTop, color });
  nextState.set(`${blockId}.3`, { ...leftTop, color });
  return { kind: "state" as const, state: nextState };
};

const applySwapMove = (move: SwapMove, state: Map<string, Block>) => {
  const block1 = state.get(move.blockId1);
  if (!block1) {
    return undefinedBlockError(move.blockId1);
  }
  const block2 = state.get(move.blockId2);
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

  const newBlock1 = { ...block1 };
  newBlock1.color = { ...block2.color };
  const newBlock2 = { ...block2 };
  newBlock2.color = { ...block1.color };

  const nextState = new Map(state);
  nextState.set(move.blockId1, newBlock1);
  nextState.set(move.blockId2, newBlock2);
  return {
    kind: "state" as const,
    state: nextState,
  };
};
