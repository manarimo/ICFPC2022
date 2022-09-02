import React, { useEffect, useMemo, useRef, useState } from "react";
import { InfoTable } from "../info-table";
import { Move } from "../parser";
import { applySingleMove, createNewState, getColor, State } from "../simulate";

interface Props {
  moves: Move[];
  width: number;
  height: number;
}
export const Canvas = ({ moves, height, width }: Props) => {
  const [turn, setTurn] = useState(moves.length);
  const result = useMemo(() => {
    return calculate(moves, width, height);
  }, [width, height, moves]);
  useEffect(() => {
    setTurn(result.states.length);
  }, [result]);
  const state = result.states[turn] as State | undefined;

  const canvasRef = useRef<HTMLCanvasElement>(null);
  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas?.getContext("2d");
    if (ctx && state) {
      ctx.fillStyle = "rgb(255,255,255)";
      ctx.fillRect(0, 0, width, height);
      for (let x = 0; x < width; x++) {
        for (let y = 0; y < height; y++) {
          const { r, g, b, a } = getColor(state, x, y);
          ctx.fillStyle = `rgba(${r},${g},${b},${a})`;
          ctx.fillRect(x, height - y - 1, 1, 1);
        }
      }
    }
  }, [canvasRef, width, height, state]);

  return (
    <div>
      <canvas
        style={{ margin: "10px" }}
        width={width}
        height={height}
        ref={canvasRef}
      ></canvas>
      <div>
        <input
          style={{ width: `${width}px` }}
          type="range"
          min="0"
          max={moves.length}
          value={turn}
          onChange={(e) => setTurn(Number.parseInt(e.target.value))}
        />
      </div>
      <div>{turn > 0 ? JSON.stringify(moves[turn - 1]) : "initialized"}</div>
      <div>
        {result.kind === "error"
          ? `${result.errorMessage}: ${JSON.stringify(result.move)}`
          : "no error"}
      </div>
      <div>
        <InfoTable cost={state?.cost ?? 0} />
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
