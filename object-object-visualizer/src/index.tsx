import React from "react";
import ReactDOM from "react-dom/client";
import "./index.css";
import App from "./App";
import { HashRouter, Link, Route, Routes } from "react-router-dom";
import { Top } from "./Top";
import { Problems } from "./Problems";
const root = ReactDOM.createRoot(
  document.getElementById("root") as HTMLElement
);
root.render(
  <React.StrictMode>
    <HashRouter>
      <nav className="navbar">
        <Link to="/" className="nav-link">
          Top
        </Link>
        <Link to="/problems" className="nav-link">
          Problems
        </Link>
        <Link to="/vis" className="nav-link">
          Visualizer
        </Link>
      </nav>
      <Routes>
        <Route path="/" element={<Top />} />
        <Route path="/vis" element={<App />} />
        <Route path="/vis/:batchName/:problemId" element={<App />} />
        <Route path="/problems" element={<Problems />} />
      </Routes>
    </HashRouter>
  </React.StrictMode>
);
