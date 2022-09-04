import React, { useEffect } from "react";
import { Viewer } from "./viewer";
import { parseProgram } from "./parser";
import { useLocalStorage } from "./hooks/useLocalStorage";
import { ProblemViewer } from "./problem-viewer";
import { useParams } from "react-router-dom";
import useSWR, { BareFetcher } from "swr";
import { manarimoFetch } from "./fetch";
import { useInitialImage, useProblemData } from "./hooks/useProblemData";
import { useInitial } from "./hooks/useInitial";
import {
  colorsToKyoproFormat,
  extractColors,
} from "./color_extractor/color_extractor";

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
  const problem = useProblemData(problemId);
  const initial = useInitial(problemId);
  const initialImage = useInitialImage(problemId);

  useEffect(() => {
    if (data !== undefined) {
      setProgram(data.code);
    }
  }, [data]);

  const programToUse = program;
  const moves = parseProgram(programToUse);
  return (
    <div style={{ display: "flex" }}>
      <div>
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
          <textarea
            style={{ width: `${WIDTH}px`, height: `${HEIGHT}px` }}
            value={colorsToKyoproFormat(extractColors(moves))}
          />
        </div>
      </div>
      <div>
        <Viewer
          problemImage={problem.data}
          moves={moves}
          width={WIDTH}
          height={HEIGHT}
          initialBlocks={initial.data?.blocks}
          initialImage={initialImage.data}
        ></Viewer>
      </div>
      <div>
        <ProblemViewer image={problem.data} />
      </div>
    </div>
  );
}

export default App;
