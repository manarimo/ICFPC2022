import React from "react";
import { Move } from "../parser";

interface Props {
  cost: number;
}

export const InfoTable = ({ cost }: Props) => {
  return (
    <table>
      <tbody>
        <tr>
          <th>操作コスト</th>
          <td>{cost}</td>
        </tr>
      </tbody>
    </table>
  );
};
