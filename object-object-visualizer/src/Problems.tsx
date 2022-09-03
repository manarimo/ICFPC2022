import { useState } from "react";
import "./Problems.css";

type ViewMode = "original" | "rgba";

export function Problems() {
  const [mode, setMode] = useState("original" as ViewMode);

  return (
    <div style={{ display: "flex", flexDirection: "column" }}>
      <select
        value={mode}
        onChange={(e) => setMode(e.currentTarget.value as ViewMode)}
      >
        <option value="original">普通の画像</option>
        <option value="rgba">RGBA分解</option>
      </select>

      {Array.from(Array(20)).map((_, i) => renderProblemPane(i + 1, mode))}
    </div>
  );
}

function renderProblemPane(problemId: number, viewMode: ViewMode) {
  return (
    <div className="problem-pane">
      {viewMode == "original" && (
        <img
          src={`http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/original/${problemId}.png`}
          className="problem-image"
        />
      )}
      {viewMode == "rgba" && (
        <div className="problem-image-table">
          <img
            src={`http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/channels/${problemId}.R.png`}
            className="problem-image"
          />
          <img
            src={`http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/channels/${problemId}.G.png`}
            className="problem-image"
          />
          <br />
          <img
            src={`http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/channels/${problemId}.B.png`}
            className="problem-image"
          />
          <img
            src={`http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/channels/${problemId}.A.png`}
            className="problem-image"
          />
        </div>
      )}
      <div className="problem-pane-control"></div>
    </div>
  );
}
