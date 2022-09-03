import { calculateSimilarity, State } from "../simulate";
import { Image } from "../types";

export const pickBestTurn = (states: State[], problemImage: Image) => {
  let minScore = calculateSimilarity(problemImage, states[0]);
  let bestTurn = 0;
  for (let turn = 1; turn < states.length; turn++) {
    const similarityScore = calculateSimilarity(problemImage, states[turn]);
    if (similarityScore + states[turn].cost < minScore) {
      minScore = similarityScore + states[turn].cost;
      bestTurn = turn;
    }
  }
  return bestTurn;
};
