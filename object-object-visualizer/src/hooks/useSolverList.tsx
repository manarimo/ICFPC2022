import useSWR from "swr";

export const useSolverList = () => {
  return useSWR<{ items: string[] }>(
    "https://gxtbs67iyup735zlzm54b6574i0pmpwl.lambda-url.us-east-1.on.aws/api/batches"
  );
};
