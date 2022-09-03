const PNG = require("pngjs/browser");

import React, { useState } from "react";
import useSWR from "swr";
import { Canvas } from "../canvas";

interface Props {
  width: number;
  height: number;
}

export const ProblemViewer = ({ width, height }: Props) => {
  // 選択された問題ID
  // 未選択の時は 0
  const [selectedProblemId, setSelectedProblemId] = useState(0);
  const problem = useProblemData(selectedProblemId);
  return (
    <div style={{ margin: "10px" }}>
      <Canvas
        width={width}
        height={height}
        getColor={() => ({ r: 0, g: 0, b: 0, a: 0 })}
      ></Canvas>
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
  const blob = await fetch(url, { mode: "no-cors" }).then((response) =>
    response.blob()
  );

  const p = new Promise((resolve, reject) => {
    const reader = new FileReader();

    reader.onload = () => {
      resolve(reader.result);
    };

    reader.onerror = () => {
      reject(reader.error);
    };

    reader.readAsBinaryString(blob);
  });

  const binaryString = await p;
  console.log(binaryString);
  new PNG({ filterType: 4 }).parse(
    binaryString,
    function (error: unknown, data: unknown) {
      console.log(error, data);
    }
  );
};

const useProblemData = (problemId: number) => {
  return useSWR(
    problemId
      ? `http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/original/${problemId}.png`
      : null,
    (url) => fetchProblem(url)
  );
};
