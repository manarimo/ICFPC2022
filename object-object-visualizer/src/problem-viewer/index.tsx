import React, { useState } from "react";
import { Canvas } from "../canvas";
import { Image } from "../types";

const ProblemCanvas = ({ image }: { image: Image }) => {
  const { width, height } = image;
  return (
    <Canvas
      width={width}
      height={height}
      getColor={(x, y) => {
        const pos = (height - y - 1) * width + x;
        return {
          r: image.r[pos],
          g: image.g[pos],
          b: image.b[pos],
          a: image.a[pos],
        };
      }}
    ></Canvas>
  );
};

interface Props {
  image?: Image;
}
export const ProblemViewer = ({ image }: Props) => {
  return (
    <div style={{ margin: "10px" }}>
      {image && <ProblemCanvas image={image} />}
    </div>
  );
};
