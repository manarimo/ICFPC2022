import commandLineArgs from "command-line-args";
import {InitialBlock} from "../src/simulate";
import {Image, loadInitialBlocks, loadProblem, moveToString} from "./util";
import {Move, parseProgram} from "../../shared/parser";
import * as fsPromises from 'fs/promises';
import {FileHandle} from 'fs/promises';
import {ChildProcessByStdio, spawn} from "child_process";
import path from "path";
import {Readable, Writable} from "stream";

interface Options {
    problemId: string;
    batchName: string;
    command: string;
}

class Input {
    constructor(readonly image: Image, readonly initialBlocks: InitialBlock[]) {}
}

class Output {
    constructor(readonly moves: Move[]) {}
}

interface Processor {
    run(input: Input, next: (input: Input) => Promise<Output>): Promise<Output>
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
        let outbuf = '';
        let emitted = false;
        proc.stdout.on('data', (buf) => {
            outbuf += buf.toString();
        });

        proc.stdout.on('end', () => {
            if (!emitted) {
                emitted = true;
                const moves = parseProgram(outbuf);
                resolve(new Output(moves));
            }
        });

        proc.on('error', (err) => {
            if (!emitted) {
                emitted = true;
                stderrFile.close();

                console.error(`Execution failed. Output of the program so far (last bytes may be dropped):\n${outbuf}`);
                reject(err);
            }
        });

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