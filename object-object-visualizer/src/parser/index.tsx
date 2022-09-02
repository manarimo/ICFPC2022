export const parseProgram = (submission: string) => {
  const operations = submission.split("\n");
  const moves = operations
    .map((line) => line.replaceAll(" ", ""))
    .filter((line) => line.length > 0)
    .map((line) => parseSingleMove(line))
    .filter((move): move is Move => !!move);
  return moves;
};

const parseSingleMove = (operation: string) => {
  if (operation.startsWith("cut")) {
    return parseCutMove(operation);
  }
  if (operation.startsWith("color")) {
    return parseColorMove(operation);
  }
  if (operation.startsWith("swap")) {
    return parseSwapMove(operation);
  }
  if (operation.startsWith("merge")) {
    return parseMergeMove(operation);
  }
  console.error(`unsupported move: ${operation}`);
};

export type LCutMove = {
  blockId: string;
  orientation: Orientation;
  lineNumber: number;
  kind: "lcut-move";
};
export type PCutMove = {
  blockId: string;
  x: number;
  y: number;
  kind: "pcut-move";
  orientation?: undefined;
  lineNumber?: undefined;
};
export type ColorMove = Exclude<ReturnType<typeof parseColorMove>, undefined>;
export type SwapMove = ReturnType<typeof parseSwapMove>;
export type MergeMove = ReturnType<typeof parseMergeMove>;

export type Move = LCutMove | PCutMove | ColorMove | SwapMove | MergeMove;

const parseCutMove = (token: string) => {
  let suffix = token.slice(3);
  const block = parseBlock(suffix);
  suffix = block.suffix;
  const blockId = block.blockId;
  const next = suffix[1];
  if (isOrientation(next)) {
    // lcut-move "cut" <block> <orientation> <line-number>
    suffix = suffix.slice(3);
    const lineNumber = parseLineNumber(suffix);
    return {
      blockId,
      orientation: next,
      lineNumber: lineNumber.lineNumber,
      kind: "lcut-move" as const,
    };
  } else {
    // pcut-move "cut" <block> <point>
    const point = parsePoint(suffix);
    if (point) {
      return {
        blockId,
        x: point.x,
        y: point.y,
        kind: "pcut-move" as const,
      };
    }
  }
};

const parseBlock = (token: string) => {
  let blockId = "";
  for (let index = 1; index < token.length; index++) {
    if (token[index] !== "]") {
      blockId += token[index];
    } else {
      const suffix = token.slice(index + 1);
      return { blockId, suffix };
    }
  }
  return { blockId, suffix: "" };
};

type Orientation = "X" | "x" | "Y" | "y";
const isOrientation = (token: string): token is Orientation => {
  return token === "X" || token === "x" || token === "Y" || token === "y";
};

const parsePoint = (token: string) => {
  const { blockId, suffix } = parseBlock(token);
  const xy = blockId.split(",");
  const x = Number.parseInt(xy[0]);
  const y = Number.parseInt(xy[1]);
  if (Number.isInteger(x) && Number.isInteger(y)) {
    return { x, y, suffix };
  } else {
    console.error(`invalid point format: ${token}`);
  }
};

const parseLineNumber = (token: string) => {
  const block = parseBlock(token);
  const lineNumber = Number.parseInt(block.blockId);
  return {
    lineNumber,
    suffix: block.suffix,
  };
};

const parseColorMove = (operation: string) => {
  let suffix = operation.slice(5);
  const block = parseBlock(suffix);
  suffix = block.suffix;
  const color = parseColor(suffix);
  if (color) {
    return {
      color: color.color,
      blockId: block.blockId,
      kind: "color-move" as const,
    };
  }
};

const parseColor = (token: string) => {
  const block = parseBlock(token);
  const rgba = block.blockId.split(",");
  const r = Number.parseInt(rgba[0]);
  const g = Number.parseInt(rgba[1]);
  const b = Number.parseInt(rgba[2]);
  const a = Number.parseInt(rgba[3]);
  if (
    Number.isInteger(r) &&
    Number.isInteger(g) &&
    Number.isInteger(b) &&
    Number.isInteger(a)
  ) {
    return {
      color: { r, g, b, a },
      suffix: block.suffix,
    };
  } else {
    console.error(`invalid color format: ${token}`);
  }
};

const parseSwapMove = (operation: string) => {
  let suffix = operation.slice(4);
  const block1 = parseBlock(suffix);
  suffix = block1.suffix;
  const block2 = parseBlock(suffix);
  return {
    blockId1: block1.blockId,
    blockId2: block2.blockId,
    kind: "swap-move" as const,
  };
};

const parseMergeMove = (operation: string) => {
  let suffix = operation.slice(5);
  const block1 = parseBlock(suffix);
  suffix = block1.suffix;
  const block2 = parseBlock(suffix);
  return {
    leftBlockId: block1.blockId,
    rightBlockId: block2.blockId,
    kind: "merge-move" as const,
  };
};
