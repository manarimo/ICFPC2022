import React from "react";
import { Link } from "react-router-dom";
import useSWR from "swr";
import { manarimoFetch } from "../fetch";

export const Solutions = () => {
  const ranking = useRanking();

  const bestScores = useBestScores();
  if (!ranking.data) {
    return <p>loading...</p>;
  }
  const bestSum = Object.entries(ranking.data!!)
    .filter((e) => !e[0].startsWith("ex"))
    .reduce((acc, item) => acc + item[1][0].score, 0);
  console.log(bestScores);
  return (
    <table>
      <tbody>
        <tr>
          <td />
          <td />
          <td />
          <td>
            <ul>
              <li>Best sum: {bestSum}</li>
            </ul>
          </td>
        </tr>
        {Object.entries(ranking.data).map(([problemId, solutions]) => {
          const orderedSolutions = [...solutions].sort(
            (a, b) => a.score - b.score
          );
          const overallBest = bestScores.get(problemId) ?? 0;
          const ourBest = orderedSolutions.length
            ? orderedSolutions[0].score
            : 0;
          return (
            <tr key={problemId}>
              <td>
                <table>
                  <tbody>
                    <tr>
                      <td>Problem</td>
                      <td style={{ textAlign: "right" }}>{problemId}</td>
                    </tr>
                    {overallBest && (
                      <tr>
                        <td>Overall Best</td>
                        <td style={{ textAlign: "right" }}>{overallBest}</td>
                      </tr>
                    )}
                    {ourBest && (
                      <tr>
                        <td>Our Best</td>
                        <td style={{ textAlign: "right" }}>{ourBest}</td>
                      </tr>
                    )}
                    {ourBest && (
                      <tr>
                        <td>Remaining</td>
                        <td style={{ textAlign: "right" }}>
                          {ourBest - overallBest}
                        </td>
                      </tr>
                    )}
                  </tbody>
                </table>
              </td>
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
                  {orderedSolutions.slice(0, 20).map((solution, i) => (
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

const useBestScores = () => {
  const bestScores = useSWR<{
    bestScores: { problemId: string; bestScore: number }[];
  }>("api/best_scores", manarimoFetch);
  console.log(bestScores.error);

  const map = new Map<string, number>();
  bestScores.data?.bestScores.forEach((bestScore) => {
    map.set(bestScore.problemId, bestScore.bestScore);
  });
  return map;
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
