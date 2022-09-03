import React, { useState } from "react";
import useSWR from "swr";
import { Canvas } from "../canvas";
const { PNG } = require("pngjs/browser");

interface ProblemCanvasProps {
  problemId: number;
}
const ProblemCanvas = ({ problemId }: ProblemCanvasProps) => {
  const problem = useProblemData(problemId);
  if (problem.data) {
    const { width, height } = problem.data;
    const d = problem.data;
    return (
      <Canvas
        width={width}
        height={height}
        getColor={(x, y) => {
          const pos = (height - y - 1) * width + x;
          return { r: d.r[pos], g: d.g[pos], b: d.b[pos], a: d.a[pos] };
        }}
      ></Canvas>
    );
  } else {
    return null;
  }
};

export const ProblemViewer = () => {
  // 選択された問題ID
  // 未選択の時は 0
  const [selectedProblemId, setSelectedProblemId] = useState(0);
  return (
    <div style={{ margin: "10px" }}>
      <ProblemCanvas problemId={selectedProblemId} />
      <div>
        <select
          onChange={(e) => {
            const problemId = Number.parseInt(e.target.value);
            if (Number.isInteger(problemId)) {
              setSelectedProblemId(problemId);
            }
          }}
        >
          <option value="0">Problem</option>
          {Array.from(Array(20)).map((v, k) => {
            const problemId = k + 1;
            return (
              <option key={problemId} value={problemId}>
                {problemId}
              </option>
            );
          })}
        </select>
      </div>
    </div>
  );
};

interface Image {
  r: Uint8Array;
  g: Uint8Array;
  b: Uint8Array;
  a: Uint8Array;
  width: number;
  height: number;
}
const fetchProblem = async (url: string) => {
  const arrayBuffer = await fetch(url).then((response) => {
    return response.arrayBuffer();
  });
  const png = new Promise((resolve, reject) => {
    new PNG({ filterType: 4 }).parse(
      arrayBuffer,
      function (error: unknown, png: any) {
        if (error != null) {
          reject("ERROR: In level image read; " + error);
        } else {
          const numPx = png.width * png.height;
          const image: Image = {
            r: new Uint8Array(numPx),
            g: new Uint8Array(numPx),
            b: new Uint8Array(numPx),
            a: new Uint8Array(numPx),
            width: png.width,
            height: png.height,
          };
          for (let px = 0; px < numPx; px++) {
            const base = px * 4;
            image.r[px] = png.data[base + 0];
            image.g[px] = png.data[base + 1];
            image.b[px] = png.data[base + 2];
            image.a[px] = png.data[base + 3];
          }
          resolve(image);
        }
      }
    );
  });
  const image = await png;
  return image as Image;
};

const useProblemData = (problemId: number) => {
  return useSWR(
    problemId
      ? `http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/original/${problemId}.png`
      : null,
    (url) => fetchProblem(url)
  );
};
function _arrayBufferToBase64(buffer: ArrayBuffer) {
  var binary = "";
  var bytes = new Uint8Array(buffer);
  var len = bytes.byteLength;
  for (var i = 0; i < len; i++) {
    binary += String.fromCharCode(bytes[i]);
  }
  return window.btoa(binary);
}
