export async function manarimoFetch<T>(path: string): Promise<T> {
  const response = await fetch(
    `https://gxtbs67iyup735zlzm54b6574i0pmpwl.lambda-url.us-east-1.on.aws/${path}`
  );
  return response.json();
}
