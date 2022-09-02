import React, { useState } from "react";
import { Canvas } from "./canvas";
import { parseProgram } from "./parser";

function App() {
  const [program, setProgram] = useState("");
  const moves = parseProgram(program);
  return (
    <div style={{ display: "flex" }}>
      <div>
        <textarea
          value={program}
          onChange={(e) => {
            setProgram(e.target.value);
          }}
        />
      </div>
      <div>
        <Canvas moves={moves} width={400} height={400}></Canvas>
      </div>
    </div>
  );
}

export default App;
