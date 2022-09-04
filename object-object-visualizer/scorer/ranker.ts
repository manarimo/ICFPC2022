// Scans all scores in the output directory and builds `output/ranking.json`
import { allSolutions, Solution, SolutionSpec } from './util';
import * as fs from 'fs';

export async function buildRaking(): Promise<Record<string, Solution[]>> {
    const solutionsByProblem: Record<string, Solution[]> = {};

    for await (let spec of allSolutions()) {
        if (!fs.existsSync(spec.scoreJsonPath())) {
            continue;
        }

        const scoreJson = JSON.parse(fs.readFileSync(spec.scoreJsonPath()).toString()) as { score: number };
        const solution = new Solution(spec.batchName, spec.problemId, scoreJson.score);
        const solutionsBuf = solutionsByProblem[solution.problemId];
        if (solutionsBuf !== undefined) {
            solutionsBuf.push(solution);
        } else {
            solutionsByProblem[solution.problemId] = [solution];
        }
    }

    // Sort solutions by ascending order of score
    for (let solutions of Object.values(solutionsByProblem)) {
        solutions.sort((a, b) => a.score - b.score);
    }

    // Fix the order of keys for consistent output
    const dict: typeof solutionsByProblem = {};
    for (let problemId of Object.keys(solutionsByProblem).sort()) {
        dict[problemId] = solutionsByProblem[problemId];
    }

    return dict;
}
