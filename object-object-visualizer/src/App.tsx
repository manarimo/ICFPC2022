import React, { useState } from "react";
import { parseProgram } from "./parser";

function App() {
  const [program, setProgram] = useState("");
  const moves = parseProgram(program);
  return (
    <div>
      <div>
        <textarea
          value={program}
          onChange={(e) => {
            setProgram(e.target.value);
          }}
        />
      </div>
      <div>{JSON.stringify(moves)}</div>
    </div>
  );
}

export default App;
