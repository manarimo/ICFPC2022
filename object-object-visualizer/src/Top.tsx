import * as React from "react";
import useSWR from "swr";
import { Link } from "react-router-dom";
import "./Top.css";
import { manarimoFetch } from "./fetch";

interface SolutionsAll {
  solutions: BatchSpec[];
}

interface BatchSpec {
  batchName: string;
  solutions: Solution[];
}

interface Solution {
  batchName: string;
  problemId: string;
}

export function Top() {
  const { data, error } = useSWR<SolutionsAll, Error>(
    "api/solution_all",
    manarimoFetch
  );

  if (data === undefined) {
    return null;
  }

  return (
    <div style={{ display: "flex", flexDirection: "column" }}>
      {data.solutions.map((batch) => renderBatchPane(batch))}
    </div>
  );
}

function renderBatchPane(batchSpec: BatchSpec) {
  return (
    <div className="batch-pane" key={batchSpec.batchName}>
      <h2 className="batch-name">{batchSpec.batchName}</h2>
      <div className="batch-solutions-list">
        {batchSpec.solutions.map((sol) => (
          <div className="batch-solution-link">
            <Link to={`/vis/${sol.batchName}/${sol.problemId}`}>
              {sol.problemId}
            </Link>
          </div>
        ))}
      </div>
    </div>
  );
}
