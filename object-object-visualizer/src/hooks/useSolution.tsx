import useSWR from "swr";

export const useSolution = (solver: string, problemId: number) => {
  return useSWR<{ code: string }>(
    `https://gxtbs67iyup735zlzm54b6574i0pmpwl.lambda-url.us-east-1.on.aws/api/solution?batch=${solver}&problemId=${problemId}`
  );
};
