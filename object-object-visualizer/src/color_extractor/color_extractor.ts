import { Move } from "../parser";

export const extractColors = (moves: Move[]) => {
  const colors: number[][] = [];
  for (const move of moves) {
    if (move.kind !== "color-move") {
      continue;
    }
    colors.push([move.color.r, move.color.g, move.color.b, move.color.a]);
  }
  return colors;
};

export const colorsToKyoproFormat = (colors: number[][]) => {
  const colorSet = new Set(
    colors.map((c) => `${c[0]} ${c[1]} ${c[2]} ${c[3]}`)
  );
  return [`${colorSet.size}`, ...Array.from(colorSet)].join("\n");
};
