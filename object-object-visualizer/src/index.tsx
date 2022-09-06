import React from "react";
import ReactDOM from "react-dom/client";
import "./index.css";
import App from "./App";
import { HashRouter, Link, Navigate, Route, Routes } from "react-router-dom";
import { Problems } from "./Problems";
import { Solutions } from "./solutions";

const root = ReactDOM.createRoot(
  document.getElementById("root") as HTMLElement
);
root.render(
  <React.StrictMode>
    <HashRouter>
      <nav className="navbar">
        <img src="/portal/manarimo.png" className="logo" />
        <Link to="/problems" className="nav-link">
          Problems
        </Link>
        <Link to="/solutions" className="nav-link">
          Solutions
        </Link>
        <Link to="/vis" className="nav-link">
          Visualizer
        </Link>
      </nav>
      <Routes>
        <Route path="/" element={<Navigate to="/solutions" />} />
        <Route path="/vis" element={<App />} />
        <Route path="/vis/:batchName/:problemId" element={<App />} />
        <Route path="/problems" element={<Problems />} />
        <Route path="/solutions" element={<Solutions />} />
      </Routes>
    </HashRouter>
  </React.StrictMode>
);
