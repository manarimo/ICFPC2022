import React, { useEffect, useMemo, useState } from "react";
import { Canvas } from "../canvas";
import { InfoTable } from "../info-table";
import { Move } from "../parser";
import {
  applySingleMove,
  calculateSimilarity,
  createNewState,
  getColor,
  State,
} from "../simulate";
import { Image } from "../types";

interface Props {
  moves: Move[];
  problemImage?: Image;
  width: number;
  height: number;
}

const unzoom = (v: number, zoom: number) => Math.floor(v / zoom);

const Picture = React.memo(
  ({
    width,
    height,
    state,
    setMousePos,
    zoom,
  }: {
    width: number;
    height: number;
    state: State | undefined;
    setMousePos: (obj: { x: number; y: number } | undefined) => void;
    zoom: number;
  }) => (
    <Canvas
      width={width * zoom}
      height={height * zoom}
      getColor={(x, y) =>
        state
          ? getColor(state, unzoom(x, zoom), unzoom(y, zoom))
          : { r: 255, g: 255, b: 255, a: 255 }
      }
      onMouseMove={(x, y) => {
        setMousePos({ x: unzoom(x, zoom), y: unzoom(y, zoom) });
      }}
    />
  )
);

export const Viewer = ({ moves, height, width, problemImage }: Props) => {
  const [turn, setTurn] = useState(0);
  const result = useMemo(() => {
    return calculate(moves, width, height);
  }, [width, height, moves]);
  useEffect(() => {
    setTurn(result.states.length - 1);
  }, [result]);
  const state = result.states[turn] as State | undefined;
  const [mousePos, setMousePos] = useState<
    { x: number; y: number } | undefined
  >();
  const [zoom, setZoom] = useState(1);
  const similarity = useMemo(() => {
    if (problemImage && state) {
      return calculateSimilarity(problemImage, state);
    } else {
      return 0;
    }
  }, [problemImage, state]);

  return (
    <div>
      <Picture
        width={width}
        height={height}
        state={state}
        setMousePos={setMousePos}
        zoom={zoom}
      />
      <div>
        <input
          style={{ width: `${width}px` }}
          type="range"
          min="0"
          max={moves.length}
          value={turn}
          onChange={(e) => setTurn(Number.parseInt(e.target.value))}
        />
        <input
          type="number"
          min={1}
          max={5}
          value={zoom}
          onChange={(e) => setZoom(Number.parseInt(e.target.value))}
        />
      </div>
      <div>
        <InfoTable
          similarity={similarity}
          cost={state?.cost ?? 0}
          turn={turn}
          move={turn > 0 ? moves[turn - 1] : undefined}
          errorMessage={
            result.kind === "error"
              ? `${result.errorMessage}: ${JSON.stringify(result.move)}`
              : undefined
          }
          mousePos={mousePos}
        />
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
