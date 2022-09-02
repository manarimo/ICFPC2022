import React from "react";
import { Viewer } from "./viewer";
import { parseProgram } from "./parser";
import { useLocalStorage } from "./hooks/useLocalStorage";
import { ProblemViewer } from "./problem-viewer";

const WIDTH = 400;
const HEIGHT = 400;

function App() {
  const [program, setProgram] = useLocalStorage("ProgramEditor", "");
  const moves = parseProgram(program);
  return (
    <div style={{ display: "flex" }}>
      <div>
        <textarea
          style={{ width: `${WIDTH}px`, height: `${HEIGHT}px` }}
          value={program}
          onChange={(e) => {
            setProgram(e.target.value);
          }}
        />
      </div>
      <div>
        <Viewer moves={moves} width={WIDTH} height={HEIGHT}></Viewer>
      </div>
      <div>
        <ProblemViewer width={WIDTH} height={HEIGHT}></ProblemViewer>
      </div>
    </div>
  );
}

export default App;
