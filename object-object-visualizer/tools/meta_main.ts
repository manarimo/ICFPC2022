import commandLineArgs from "command-line-args";
import {loadInitialBlocks, loadProblem, moveToString} from "./util";
import * as fsPromises from 'fs/promises';
import {FileHandle} from 'fs/promises';
import {ChildProcessByStdio, spawn} from "child_process";
import path from "path";
import {Readable, Writable} from "stream";
import {Input, Output} from "./metaprocessor";
import {parseProgram} from "../src/parser";

interface Options {
    problemId: string;
    batchName: string;
    command: string;
}

class ProcessRunner {
    constructor(private command: string, private outDir: string, private problemId: string) {
    }

    async run(input: Input): Promise<Output> {
        const stderrFile = await fsPromises.open(`${this.outDir}/${this.problemId}_stderr.txt`, 'w');

        const aiDir = path.dirname(this.command);
        const proc = spawn(this.command, {
            stdio: ['pipe', 'pipe', stderrFile.createWriteStream()],
        });

        return new Promise((resolve, reject) => {
            try {
                this.promiseBody(input, stderrFile, proc, resolve, reject);
            } catch (e) {
                reject(e);
            }
        });
    }

    private promiseBody(
        input: Input,
        stderrFile: FileHandle,
        proc: ChildProcessByStdio<Writable, Readable, null>,
        resolve: (o: Output) => void,
        reject: (err: any) => void
    ) {
        let emitted = false;

        // Accumulate output to a buffer as the data come to stdout
        let outbuf = '';
        proc.stdout.on('data', (buf) => {
            outbuf += buf.toString();
        });

        // Have read everything from stdout; meaning all output is there. Resolve the promise with parsed output.
        proc.stdout.on('end', () => {
            if (!emitted) {
                emitted = true;
                const moves = parseProgram(outbuf);
                resolve(new Output(moves));
            }
        });

        // On execution failure, reject promise to abort execution
        proc.on('error', (err) => {
            if (!emitted) {
                emitted = true;
                stderrFile.close();

                console.error(`Execution failed. Output of the program so far (last bytes may be dropped):\n${outbuf}`);
                reject(err);
            }
        });

        // When a process exists, we don't need stderr anymore.
        proc.on('exit', () => {
            stderrFile.close();
        });

        // Write out input in the kyopro format
        proc.stdin.write(`${input.image.width} ${input.image.height}\n`);
        for (let buf of [input.image.r, input.image.g, input.image.b, input.image.a]) {
            for (let px = 0; px < input.image.width * input.image.height; px++) {
                if (px > 0) {
                    proc.stdin.write(' ');
                }
                proc.stdin.write(String(input.image.r[px]));
                if (px % input.image.width == input.image.width - 1) {
                    proc.stdin.write("\n");
                }
            }
        }
        proc.stdin.write(`${input.initialBlocks.length}\n`);
        for (let block of input.initialBlocks) {
            proc.stdin.write(`${block.blockId}\n`);
            proc.stdin.write(`${block.bottomLeft.join(' ')}\n`);
            proc.stdin.write(`${block.topRight.join(' ')}\n`);
            proc.stdin.write(`${block.color.join(' ')}\n`);
        }
        proc.stdin.end();
    }
}

async function main(options: Options) {
    const outDir = `../../output/${options.batchName}`;
    await fsPromises.mkdir(outDir, { recursive: true });

    // Load input from files
    const problemImage = await loadProblem(`../../problem/original/${options.problemId}.png`);
    const initialBlocks = await loadInitialBlocks(`../../problem/original/${options.problemId}.initial.json`);
    const input = new Input(problemImage, initialBlocks) ;

    // Run process
    const processRunner = new ProcessRunner(options.command, outDir, options.problemId);
    const output = await processRunner.run(input);

    // Write out moves to the file
    const code = output.moves.map(moveToString);
    await fsPromises.writeFile(`${outDir}/${options.problemId}.isl`, code.join('\n'));
}

const options: Options = commandLineArgs([
    { name: 'problemId', type: String },
    { name: 'batchName', type: String },
    { name: 'command', type: String },
]) as Options;
main(options);