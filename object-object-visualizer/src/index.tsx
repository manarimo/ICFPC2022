import React from "react";
import ReactDOM from "react-dom/client";
import "./index.css";
import App from "./App";
import { BrowserRouter, HashRouter, Route, Routes } from "react-router-dom";
import { Top } from "./Top";
const root = ReactDOM.createRoot(
  document.getElementById("root") as HTMLElement
);
root.render(
  <React.StrictMode>
    <HashRouter>
      <Routes>
        <Route path="/" element={<Top />} />
        <Route path="/vis" element={<App />} />
      </Routes>
    </HashRouter>
  </React.StrictMode>
);
