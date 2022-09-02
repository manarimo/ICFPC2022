import React, { useState } from "react";
import { Viewer } from "./viewer";
import { parseProgram } from "./parser";
import { useLocalStorage } from "./hooks/useLocalStorage";

function App() {
  const [program, setProgram] = useLocalStorage("ProgramEditor", "");
  const moves = parseProgram(program);
  return (
    <div style={{ display: "flex" }}>
      <div>
        <textarea
          style={{ width: "400px", height: "400px" }}
          value={program}
          onChange={(e) => {
            setProgram(e.target.value);
          }}
        />
      </div>
      <div>
        <Viewer moves={moves} width={400} height={400}></Viewer>
      </div>
    </div>
  );
}

export default App;
