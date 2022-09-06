export async function manarimoFetch<T>(path: string): Promise<T> {
  let realPath = path;
  if (path.startsWith("api/")) {
    realPath = path.substring(4);
  }
  const response = await fetch(
    `http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/output/${realPath}.json`
  );
  return response.json();
}
