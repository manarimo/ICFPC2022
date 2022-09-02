import React, { useCallback, useEffect, useRef } from "react";

interface Props {
  width: number;
  height: number;
  getColor: (
    x: number,
    y: number
  ) => { r: number; g: number; b: number; a: number };
  onMouseMove?: (x: number, y: number) => void;
}
export const Canvas = ({ width, height, getColor, onMouseMove }: Props) => {
  const handleMouseMove = useCallback(
    (x: number, y: number) => {
      if (onMouseMove) {
        onMouseMove(x, height - y);
      }
    },
    [onMouseMove, height]
  );
  const canvasRef = useRef<HTMLCanvasElement>(null);
  useEffect(() => {
    const canvas = canvasRef.current;
    const ctx = canvas?.getContext("2d");
    if (ctx) {
      canvas?.addEventListener(
        "mousemove",
        (e) => {
          const rect = canvas.getBoundingClientRect();
          const { left, top } = rect;
          const x = Math.floor(e.clientX - left);
          const y = Math.floor(e.clientY - top);
          handleMouseMove(x, y);
        },
        false
      );
      ctx.clearRect(0, 0, width, height);
      for (let x = 0; x < width; x++) {
        for (let y = 0; y < height; y++) {
          const { r, g, b, a } = getColor(x, y);
          ctx.fillStyle = `rgba(${r},${g},${b},${a})`;
          ctx.fillRect(x, height - y - 1, 1, 1);
        }
      }
    }
  }, [canvasRef, width, height, getColor, handleMouseMove]);
  return (
    <canvas
      style={{ margin: "10px" }}
      width={width}
      height={height}
      ref={canvasRef}
    ></canvas>
  );
};
