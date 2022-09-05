import {loadInitialBlocks, loadMoves, loadPalette, loadProblem, moveToString, topNSolutions} from "./util";
import {Merger} from "./merger";
import * as fs from "fs";
import {Input, Output} from "./metaprocessor";
import * as fsPromises from 'fs/promises';

const MERGER_TO_PLAIN = {
    "26":"5",
    "27":"2",
    "28":"10",
    "29":"18",
    "30":"11",
    "31":"24",
    "32":"9",
    "33":"15",
    "34":"7",
    "35":"16",
};

const PLAIN_TO_MERGER: Record<string, string> = {}
for (const entry of Object.entries(MERGER_TO_PLAIN)) {
    PLAIN_TO_MERGER[entry[1]] = entry[0];
}

async function main() {
    const merger = new Merger();
    await fsPromises.mkdir(`../../output/merger-combined`, { recursive: true });

    for await (const solution of topNSolutions(1)) {
        const mergerProblemId = PLAIN_TO_MERGER[solution.problemId];
        if (mergerProblemId === undefined) {
            continue;
        }
        console.log(`combining ${solution.problemId} -> ${mergerProblemId}`);

        // Load input from files
        const problemImage = await loadProblem(`../../problem/original/${mergerProblemId}.png`);
        const initialBlocks = await loadInitialBlocks(`../../problem/original/${mergerProblemId}.initial.json`);
        const initialImagePath = `../../problem/original_initial/${mergerProblemId}.initial.png`;
        const initialProblemImage = fs.existsSync(initialImagePath) ? await loadProblem(initialImagePath) : null;
        const palettePath = `../../problem/original/${mergerProblemId}.palette.txt`;
        const palette = await loadPalette(palettePath);

        const input = new Input(problemImage, initialBlocks, initialProblemImage, palette);
        const moves = await loadMoves(`../../output/${solution.batchName}/${solution.problemId}.isl`);
        const output = await merger.run(input, async () => new Output(moves));

        output.moves.unshift({ kind: 'comment-move', comment: `merger-solver: based on ${solution.batchName}/${solution.problemId}` });
        const code = output.moves.map(moveToString).join("\n");
        await fsPromises.writeFile(`../../output/merger-combined/${mergerProblemId}.isl`, code);
    }
}

main();