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

type Move =
  | ReturnType<typeof parseCutMove>
  | ReturnType<typeof parseColorMove>
  | ReturnType<typeof parseSwapMove>
  | ReturnType<typeof parseMergeMove>;

const parseCutMove = (token: string) => {
  let suffix = token.slice(3);
  const block = parseBlock(suffix);
  suffix = block.suffix;
  const blockId = block.blockId;
  const next = suffix[0];
  if (isOrientation(next)) {
    // lcut-move "cut" <block> <orientation> <line-number>
    suffix = suffix.slice(1);
    const lineNumber = parseLineNumber(suffix);
    return {
      blockId,
      orientation: next,
      lineNumber,
      kind: "pcut-move" as const,
    };
  } else {
    // pcut-move "cut" <block> <point>
    const block = parseBlock(suffix);
    suffix = block.suffix;
    const point = parsePoint(suffix);
    return {
      blockId: block.blockId,
      x: point.x,
      y: point.y,
      kind: "lcut-move" as const,
    };
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
  return { x, y, suffix };
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
  return {
    r: color.r,
    g: color.g,
    b: color.b,
    a: color.a,
    blockId: block.blockId,
    kind: "color-move" as const,
  };
};

const parseColor = (token: string) => {
  const block = parseBlock(token);
  const rgba = block.blockId.split(",");
  const r = Number.parseInt(rgba[0]);
  const g = Number.parseInt(rgba[1]);
  const b = Number.parseInt(rgba[2]);
  const a = Number.parseInt(rgba[3]);
  return {
    r,
    g,
    b,
    a,
    suffix: block.suffix,
  };
};

const parseSwapMove = (operation: string) => {
  let suffix = operation.slice(4);
  const block1 = parseBlock(suffix);
  suffix = block1.suffix;
  const block2 = parseBlock(suffix);
  return {
    leftBlockId: block1.blockId,
    rightBlockId: block2.blockId,
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
