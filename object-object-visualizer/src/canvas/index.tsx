import React, { useEffect, useMemo, useState } from "react";
import { Move } from "../parser";
import { applySingleMove, createNewState, getColor } from "./simulate";

interface Props {
  moves: Move[];
  width: number;
  height: number;
}
export const Canvas = ({ moves, height, width }: Props) => {
  const [turn, setTurn] = useState(moves.length);
  const states = useMemo(() => {
    return calculate(moves, width, height);
  }, [width, height, moves]);
  useEffect(() => {
    setTurn(states.states.length);
  }, [states]);
  const state = states.states[turn];
  return (
    <div>
      <svg width={height} height={width}>
        {Array.from(Array(width), (v, x) => {
          return Array.from(Array(height), (w, y) => {
            if (!state) {
              return (
                <rect
                  key={`${x}-${y}`}
                  x={x}
                  y={height - y - 1}
                  width={1}
                  height={1}
                  fill={`rgba(0,0,0,0)`}
                />
              );
            }
            const { r, g, b, a } = getColor(state, x, y);
            return (
              <rect
                key={`${x}-${y}`}
                x={x}
                y={height - y - 1}
                width={1}
                height={1}
                fill={`rgba(${r},${g},${b},${a})`}
              />
            );
          });
        })}
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
      <div>{turn > 0 ? JSON.stringify(moves[turn - 1]) : "initialized"}</div>
      <div>
        {states.kind === "error"
          ? `${states.errorMessage}: ${JSON.stringify(states.move)}`
          : "no error"}
      </div>
    </div>
  );
};

const calculate = (moves: Move[], width: number, height: number) => {
  let state = createNewState(width, height);
  const states = [state];
  for (const move of moves) {
    const result = applySingleMove(move, state);
    if (result.kind === "error") {
      return {
        kind: "error" as const,
        errorMessage: result.errorMessage,
        move,
        states,
      };
    }
    state = result.state;
    states.push(state);
  }

  return { kind: "states" as const, states };
};
