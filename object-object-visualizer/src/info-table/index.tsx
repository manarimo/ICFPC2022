import React from "react";
import { Move } from "../parser";

interface Props {
  cost: number;
  errorMessage?: string;
  move?: Move;
  turn: number;
  mousePos?: { x: number; y: number };
}

export const InfoTable = ({
  cost,
  errorMessage,
  move,
  turn,
  mousePos,
}: Props) => {
  return (
    <table style={{ width: "400px" }}>
      <tbody>
        <tr>
          <th>操作回数</th>
          <td>{turn}</td>
        </tr>
        <tr>
          <th>累積操作コスト</th>
          <td>{cost}</td>
        </tr>
        <tr>
          <th>座標</th>
          <td>{mousePos ? `${mousePos.x}, ${mousePos.y}` : ""}</td>
        </tr>
        <tr>
          <th>Move</th>
          <td style={{ wordBreak: "break-all" }}>
            {move ? JSON.stringify(move) : ""}
          </td>
        </tr>
        <tr>
          <th>エラー</th>
          <td>{errorMessage ?? ""}</td>
        </tr>
      </tbody>
    </table>
  );
};
