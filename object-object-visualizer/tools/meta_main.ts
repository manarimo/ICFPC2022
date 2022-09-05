import commandLineArgs from 'command-line-args';
import { loadInitialBlocks, loadProblem, moveToString, Image, createImageFromBlocks, loadPalette } from './util';
import * as fsPromises from 'fs/promises';
import { FileHandle } from 'fs/promises';
import * as fs from 'fs';
import { ChildProcessByStdio, spawn } from 'child_process';
import path from 'path';
import { Readable, Writable } from 'stream';
import { Input, Output, Processor } from './metaprocessor';
import { parseProgram } from '../src/parser';
import { CopyrightPlugin } from './plugin/copyright_plugin';
import { RotationSpec, Rotator } from './rotator';
import { TurnPicker } from './plugin/turn_picker';
import { Merger } from './merger';
import { Shifter } from './shifter';
import { ManualShifter } from './manual_shifter';
import { FreelunchPlugin } from './plugin/freelunch';

interface Options {
    problemId: string;
    batchName: string;
    command: string;

    // Turn picker plugin
    turnPicker?: boolean;

    // Shifter plugin
    split?: number;
    splitAxis?: 'X' | 'Y';
    msplit?: string;

    // Rotator plugin
    rotate?: number;
    flip?: boolean;
    rotator?: string;
}

const ROTATION_PRESET: Record<string, RotationSpec[]> = {
    'kawatea-dp-special': [
        { rotate: 0, flip: false },
        { rotate: 1, flip: false },
        { rotate: 2, flip: false },
        { rotate: 3, flip: false },
    ],
};

class ProcessRunner {
    constructor(private command: string, private outDir: string, private problemId: string) {}

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
        reject: (err: any) => void,
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
        proc.stdin.on('error', (e) => {
            console.warn(`${this.problemId}: io error during kyopro format generation`, e);
        });

        function writeImage(image: Image) {
            for (let buf of [image.r, image.g, image.b, image.a]) {
                for (let px = 0; px < image.width * image.height; px++) {
                    if (px % image.width > 0) {
                        proc.stdin.write(' ');
                    }
                    proc.stdin.write(String(buf[px]));
                    if (px % image.width == image.width - 1) {
                        proc.stdin.write('\n');
                    }
                }
            }
        }

        proc.stdin.write(`${input.image.width} ${input.image.height}\n`);
        writeImage(input.image);

        proc.stdin.write(`${input.initialBlocks.length}\n`);
        for (let block of input.initialBlocks) {
            proc.stdin.write(`${block.blockId}\n`);
            proc.stdin.write(`${block.bottomLeft.join(' ')}\n`);
            proc.stdin.write(`${block.topRight.join(' ')}\n`);
            proc.stdin.write(`${(block.color ?? [0, 0, 0, 255]).join(' ')}\n`); // this field is deprecated.
        }

        let currentGlobalCounter = input.initialBlocks.map((block) => parseInt(block.blockId.split('.')[0])).reduce((x, y) => Math.max(x, y), 0);
        proc.stdin.write(`${currentGlobalCounter}\n`);
        let initialImage: Image;
        if (input.initialImage !== null) {
            // v2 cost
            proc.stdin.write(`2 3 5 3 1\n`);
            // initialize the canvas with initialImage
            initialImage = input.initialImage;
        } else {
            // v1 cost
            proc.stdin.write(`7 10 5 3 1\n`);
            // initialize the canvas with blocks
            initialImage = createImageFromBlocks(input.initialBlocks);
        }
        writeImage(initialImage);

        proc.stdin.write(input.palette);
        proc.stdin.write('\n');

        proc.stdin.end();
    }
}

function wrap(sink: (input: Input) => Promise<Output>, processor: Processor): (input: Input) => Promise<Output> {
    return (input: Input) => processor.run(input, sink);
}

function buildPipeline(options: Options, processRunner: ProcessRunner): (input: Input) => Promise<Output> {
    // ***** Plugins are applied in the reversed order as appeared in the code *****

    // Sink
    let pipeline = (input: Input) => processRunner.run(input);

    // Add turn-picker plugin
    if (options.turnPicker == true) {
        pipeline = wrap(pipeline, new TurnPicker());
    }

    // Add shifter plugin
    if (options.split !== undefined) {
        const axis = options.splitAxis ?? 'X';
        pipeline = wrap(pipeline, new Shifter(options.split, axis));
    }

    // Add shifter plugin
    if (options.msplit !== undefined) {
        pipeline = wrap(pipeline, new ManualShifter(options.msplit));
    }

    // Add freelunch plugin
    pipeline = wrap(pipeline, new FreelunchPlugin());

    // Add rotator plugin
    if (options.rotate || options.flip) {
        const rotate = options.rotate || 0;
        const flip = options.flip == true;
        pipeline = wrap(pipeline, new Rotator([{ rotate, flip }]));
    } else if (options.rotator != undefined) {
        const preset = ROTATION_PRESET[options.rotator];
        if (preset === undefined) {
            throw new Error(`Unrecognized rotator preset: ${options.rotator}`);
        }
        pipeline = wrap(pipeline, new Rotator(preset));
    }

    // Add merger plugin
    pipeline = wrap(pipeline, new Merger());

    // Add copyright plugin
    pipeline = wrap(pipeline, new CopyrightPlugin());

    return pipeline;
}

async function main(options: Options) {
    const outDir = `../../output/${options.batchName}`;
    await fsPromises.mkdir(outDir, { recursive: true });

    // Build pipeline
    const processRunner = new ProcessRunner(options.command, outDir, options.problemId);
    const pipeline = buildPipeline(options, processRunner);

    // Load input from files
    const problemImage = await loadProblem(`../../problem/original/${options.problemId}.png`);
    const initialBlocks = await loadInitialBlocks(`../../problem/original/${options.problemId}.initial.json`);
    const initialImagePath = `../../problem/original_initial/${options.problemId}.initial.png`;
    const initialProblemImage = fs.existsSync(initialImagePath) ? await loadProblem(initialImagePath) : null;
    const palettePath = `../../problem/original/${options.problemId}.palette.txt`;
    const palette = await loadPalette(palettePath);

    const input = new Input(problemImage, initialBlocks, initialProblemImage, palette);

    // Run process
    const output = await pipeline(input);

    // Write out moves to the file
    const code = output.moves.map(moveToString);
    await fsPromises.writeFile(`${outDir}/${options.problemId}.isl`, code.join('\n'));
}

const options: Options = commandLineArgs([
    { name: 'problemId', type: String },
    { name: 'batchName', type: String },
    { name: 'command', type: String },
    { name: 'turnPicker', type: Boolean },
    { name: 'split', type: Number },
    { name: 'splitAxis', type: String },
    { name: 'msplit', type: String },
    { name: 'rotate', type: Number },
    { name: 'flip', type: Boolean },
    { name: 'rotator', type: String },
]) as Options;

if (options.rotator != undefined && (options.rotate != undefined || options.flip != undefined)) {
    console.error("Can't use --rotator with --rotate or --flip");
    process.exit(1);
}

main(options);
