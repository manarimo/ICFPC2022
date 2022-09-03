import React, { useEffect } from "react";
import { Viewer } from "./viewer";
import { parseProgram } from "./parser";
import { useLocalStorage } from "./hooks/useLocalStorage";
import { ProblemViewer } from "./problem-viewer";
import { useParams } from "react-router-dom";
import useSWR, { BareFetcher } from "swr";
import { manarimoFetch } from "./fetch";

const WIDTH = 400;
const HEIGHT = 400;

interface Solution {
  code: string;
}

function fetcher(
  batchName?: string,
  problemId?: string
): BareFetcher<Solution> {
  if (batchName !== undefined && problemId !== undefined) {
    return () =>
      manarimoFetch(`api/solution?batch=${batchName}&problemId=${problemId}`);
  } else {
    return () => {
      throw new Error("Simulating network failure");
    };
  }
}

function App() {
  const { batchName, problemId } = useParams();
  const { data, error } = useSWR<Solution>(
    `${batchName}-${problemId}`,
    fetcher(batchName, problemId)
  );
  const [program, setProgram] = useLocalStorage("ProgramEditor", "");

  useEffect(() => {
    if (data !== undefined) {
      setProgram(data.code);
    }
  }, [data]);

  const programToUse = data?.code ?? program;
  const moves = parseProgram(programToUse);
  return (
    <div style={{ display: "flex" }}>
      <div>
        <textarea
          style={{ width: `${WIDTH}px`, height: `${HEIGHT}px` }}
          value={programToUse}
          onChange={(e) => {
            setProgram(e.target.value);
          }}
        />
      </div>
      <div>
        <Viewer moves={moves} width={WIDTH} height={HEIGHT}></Viewer>
      </div>
      <div>
        <ProblemViewer></ProblemViewer>
      </div>
    </div>
  );
}

export default App;
