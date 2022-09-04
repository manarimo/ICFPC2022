import commandLineArgs from 'command-line-args';
import { loadInitialBlocks, loadProblem, moveToString, SolutionSpec } from '../util';
import { Merger } from './merger';

interface Options {
    problemId: string;
}

async function main(options: Options) {
    const spec = new SolutionSpec('dummy', options.problemId);
    const problem = await loadProblem(spec.problemImagePath());
    const initialBlocks = await loadInitialBlocks(spec.initialBlocksPath());
    const merger = Merger.create(problem, initialBlocks);
    const [moves, finalId] = merger.generateMoves();
    console.log(moves.map(moveToString).join('\n'));
}

const options: Options = commandLineArgs([{ name: 'problemId', alias: 'p', type: String }]) as Options;
main(options);
