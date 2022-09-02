import React, { useEffect, useMemo, useState } from "react";
import { Move } from "../parser";
import { applySingleMove } from "./simulate";

interface Props {
  moves: Move[];
  width: number;
  height: number;
}
export const Canvas = ({ moves, height, width }: Props) => {
  const [turn, setTurn] = useState(moves.length);
  useEffect(() => {
    setTurn(moves.length);
  }, [moves]);
  const state = useMemo(() => {
    return calculate(moves.slice(0, turn), width, height);
  }, [turn, width, height, moves]);
  return (
    <div>
      <svg width={height} height={width}>
        {state.kind === "error" ? (
          <text>{state.errorMessage}</text>
        ) : (
          <>
            {Array.from(state.state).map(([blockId, block]) => {
              const dx = block.x2 - block.x1;
              const dy = block.y2 - block.y1;
              return (
                <rect
                  key={blockId}
                  x={block.x1}
                  width={dx}
                  y={height - block.y1 - dy}
                  height={dy}
                  fill={`rgba(${block.color.r},${block.color.g},${block.color.b},${block.color.a})`}
                />
              );
            })}
          </>
        )}
      </svg>
      <div>
        <input
          type="range"
          min="0"
          max={moves.length}
          value={turn}
          onChange={(e) => setTurn(Number.parseInt(e.target.value))}
        />
      </div>
      <div>{turn > 0 ? JSON.stringify(moves[turn - 1]) : ""}</div>
    </div>
  );
};

const calculate = (moves: Move[], width: number, height: number) => {
  let state = new Map([
    [
      "0",
      {
        x1: 0,
        y1: 0,
        x2: width,
        y2: height,
        color: { r: 0, g: 0, b: 0, a: 0 },
      },
    ],
  ]);

  for (const move of moves) {
    const result = applySingleMove(move, state);
    if (result.kind === "error") {
      return result;
    }
    state = result.state;
  }

  return { kind: "state" as const, state };
};
