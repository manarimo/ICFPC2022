import useSWR from "swr";
import { Image } from "../types";
const { PNG } = require("pngjs/browser");

export const useProblemData = (problemId: string | undefined) => {
  return useSWR(
    problemId
      ? `http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/original/${problemId}.png`
      : null,
    (url) => fetchProblem(url)
  );
};

export const useInitialImage = (problemId: string | undefined) => {
  return useSWR(
    problemId && problemId in ["36", "37", "38", "39", "40"]
      ? `http://icfpc2022-manarimo.s3-website-us-east-1.amazonaws.com/problem/original_initial/${problemId}.png`
      : null,
    (url) => fetchProblem(url)
  );
};

const fetchProblem = async (url: string) => {
  const arrayBuffer = await fetch(url).then((response) => {
    return response.arrayBuffer();
  });
  const png = new Promise((resolve, reject) => {
    new PNG({ filterType: 4 }).parse(
      arrayBuffer,
      function (error: unknown, png: any) {
        if (error != null) {
          reject("ERROR: In level image read; " + error);
        } else {
          const numPx = png.width * png.height;
          const image: Image = {
            r: new Uint8Array(numPx),
            g: new Uint8Array(numPx),
            b: new Uint8Array(numPx),
            a: new Uint8Array(numPx),
            width: png.width,
            height: png.height,
          };
          for (let px = 0; px < numPx; px++) {
            const base = px * 4;
            image.r[px] = png.data[base + 0];
            image.g[px] = png.data[base + 1];
            image.b[px] = png.data[base + 2];
            image.a[px] = png.data[base + 3];
          }
          resolve(image);
        }
      }
    );
  });
  const image = await png;
  return image as Image;
};
