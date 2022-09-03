import React from "react";
import { Link } from "react-router-dom";
import useSWR from "swr";

export const Solutions = () => {
  const ranking = useRanking();
  if (!ranking.data) {
    return <p>loading...</p>;
  }

  return (
    <table>
      <tbody>
        {Object.entries(ranking.data).map(([problemId, solutions]) => {
          const orderedSolutions = [...solutions].sort(
            (a, b) => a.score - b.score
          );
          return (
            <tr key={problemId}>
              <td>Problem: {problemId}</td>
              <td>
                <img
                  src={`http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/original/${problemId}.png`}
                ></img>
              </td>
              <td>
                {orderedSolutions.length > 0 ? (
                  <>
                    <img
                      src={`http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/output/${orderedSolutions[0].batchName}/${problemId}.png`}
                    ></img>
                  </>
                ) : null}
              </td>
              <td>
                <ul>
                  {orderedSolutions.map((solution, i) => (
                    <li
                      key={solution.batchName}
                      style={{ fontWeight: i === 0 ? "bold" : "normal" }}
                    >
                      <Link
                        to={`/vis/${solution.batchName}/${solution.problemId}`}
                      >
                        {solution.score}: {solution.batchName}
                      </Link>
                    </li>
                  ))}
                </ul>
              </td>
            </tr>
          );
        })}
      </tbody>
    </table>
  );
};

const useRanking = () => {
  return useSWR<Ranking>(
    "http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/output/ranking.json",
    (url) =>
      fetch(url)
        .then((response) => response.json())
        .then((response) => response as Ranking)
  );
};

type Ranking = {
  [problemId: string]: {
    batchName: string;
    problemId: string;
    score: number;
  }[];
};
