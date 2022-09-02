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

const problemFetcher = async (url: string) => {
  const body = await fetch(url, { mode: "no-cors" }).then((response) =>
    response.text()
  );
  const values = body
    .split("\n")
    .flatMap((line) => line.split(" "))
    .map((value) => Number.parseInt(value.trim()))
    .filter((value) => Number.isInteger(value));
  const width = values[0];
  const height = values[1];
  const r = new Uint8Array(width * height);
  const g = new Uint8Array(width * height);
  const b = new Uint8Array(width * height);
  const a = new Uint8Array(width * height);
  for (let i = 0; i < width * height; i++) {
    r[i] = values[i + 2];
    g[i] = values[i + 2 + width * height];
    b[i] = values[i + 2 + width * height * 2];
    a[i] = values[i + 2 + width * height * 3];
  }
  return { r, g, b, a };
};

const useProblemData = (problemId: number) => {
  return useSWR(
    problemId
      ? `http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/plaintext/${problemId}.txt`
      : null,
    (url) => problemFetcher(url)
  );
};
