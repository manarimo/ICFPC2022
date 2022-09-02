import * as React from "react";
import useSWR from "swr";
import { Link } from "react-router-dom";

async function fetcher<T>(path: string): Promise<T> {
  const response = await fetch(
    `https://gxtbs67iyup735zlzm54b6574i0pmpwl.lambda-url.us-east-1.on.aws/${path}`
  );
  return response.json();
}

interface Batches {
  items: string[];
}

export function Top() {
  const { data, error } = useSWR<Batches, Error>("api/batches", fetcher);

  if (data === undefined) {
    return null;
  }

  return (
    <div style={{ display: "flex" }}>
      <table style={{ margin: "auto" }}>
        <thead>
          <tr>
            <th>Batch name</th>
          </tr>
        </thead>
        <tbody>
          {data.items.map((item) => (
            <tr key={item}>
              <td>{item}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}
