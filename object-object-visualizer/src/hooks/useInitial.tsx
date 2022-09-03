import useSWR from "swr";
import { InitialBlock } from "../simulate";

type Initial = {
  width: number;
  height: number;
  blocks: InitialBlock[];
};

export const useInitial = (problemId: string | undefined) => {
  return useSWR(
    problemId
      ? `http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/original/${problemId}.initial.json`
      : null,
    (url) => fetchJson(url)
  );
};

const fetchJson = async (url: string) => {
  return fetch(url).then((response) => response.json() as Promise<Initial>);
};
