import React from "react";
import ReactDOM from "react-dom/client";
import "./index.css";
import App from "./App";
import { HashRouter, Route, Routes } from "react-router-dom";
import { Top } from "./Top";
import { Problems } from "./Problems";
const root = ReactDOM.createRoot(
  document.getElementById("root") as HTMLElement
);
root.render(
  <React.StrictMode>
    <HashRouter>
      <Routes>
        <Route path="/" element={<Top />} />
        <Route path="/vis" element={<App />} />
        <Route path="/vis/:batchName/:problemId" element={<App />} />
        <Route path="/problems" element={<Problems />} />
      </Routes>
    </HashRouter>
  </React.StrictMode>
);
