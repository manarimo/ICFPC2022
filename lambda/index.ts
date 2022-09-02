import { APIGatewayProxyResult, Context, SQSEvent } from 'aws-lambda';
import { S3 } from 'aws-sdk';
import * as fsPromises from 'fs/promises';
import { GetObjectOutput } from 'aws-sdk/clients/s3';
import { spawn } from 'child_process';
import AdmZip from 'adm-zip';
import { Stream } from 'stream';

interface Args {
    aiName: string;
    batchName: string;
    problemId: string;
}

function getObject(path: string): Promise<GetObjectOutput> {
    const s3 = new S3();
    return new Promise((resolve, reject) => {
        s3.getObject({ Bucket: 'icfpc2022-manarimo', Key: path }, (err, data) => {
            if (err) {
                reject(err);
            } else {
                resolve(data);
            }
        });
    });
}

function uploadObject(body: Stream, s3Path: string): Promise<void> {
    const s3 = new S3();
    return new Promise((resolve, reject) => {
        s3.upload({ Bucket: 'icfpc2022-manarimo', Key: s3Path, Body: body }, (err, data) => {
            if (err) {
                reject(err);
            } else {
                resolve();
            }
        });
    });
}

async function downloadAi(aiDir: string, aiName: string): Promise<void> {
    const fileHandle = await fsPromises.open(`${aiDir}/ai.zip`, 'w', 0o755);
    const obj = await getObject(`ai/${aiName}.zip`);
    if (obj.Body == null) {
        throw new Error(`Failed to download AI archive: ${aiName}`);
    }
    await fileHandle.write(obj.Body as Buffer);
    await fileHandle.close();

    new AdmZip(`${aiDir}/ai.zip`).extractAllTo(aiDir, true);
}

async function runAi(aiDir: string, problemId: string, outDir: string): Promise<string[]> {
    const envVars = {
        PROBLEM_ID: problemId,
        PROBLEM_DIR: '/tmp/problem',
        OUTPUT_FILE: `${outDir}/${problemId}.txt`,
    };

    const stdoutFile = await fsPromises.open(`${outDir}/${problemId}_stdout.txt`, 'w');
    const stderrFile = await fsPromises.open(`${outDir}/${problemId}_stderr.txt`, 'w');

    await fsPromises.chmod(`${aiDir}/run.sh`, 0o755);
    const proc = spawn(`${aiDir}/run.sh`, {
        cwd: aiDir,
        stdio: [null, stdoutFile.createWriteStream(), stderrFile.createWriteStream()],
        env: envVars,
    });
    return new Promise((resolve, reject) => {
        let emitted = false;
        proc.on('error', (err) => {
            if (!emitted) {
                emitted = true;
                stdoutFile.close();
                stderrFile.close();
                reject(err);
            }
        });
        proc.on('exit', () => {
            if (!emitted) {
                emitted = true;
                stdoutFile.close();
                stderrFile.close();
                resolve([`${problemId}_stdout.txt`, `${problemId}_stderr.txt`, `${problemId}.txt`]);
            }
        });
    });
}

export async function handler(event: SQSEvent, context: Context): Promise<APIGatewayProxyResult> {
    console.log(`Event: ${JSON.stringify(event, null, 2)}`);
    if (event.Records.length != 1) {
        return {
            statusCode: 400,
            body: 'This function cannot process more than 1 message at once',
        };
    }
    const args: Args = JSON.parse(event.Records[0].body);

    const aiDir = `/tmp/ai/${args.aiName}`;
    const outDir = '/tmp/output';
    await fsPromises.mkdir(aiDir, { recursive: true });
    await fsPromises.mkdir(outDir, { recursive: true });
    await downloadAi(aiDir, args.aiName);

    const files = await runAi(aiDir, args.problemId, outDir);
    const promises = files.map(async (f) => {
        console.log(`Uploading ${f}`);
        const handle = await fsPromises.open(`${outDir}/${f}`, 'r');
        await uploadObject(handle.createReadStream(), `output/${args.batchName}/${f}`);
        await handle.close();
    });
    await Promise.all(promises);

    return {
        statusCode: 200,
        body: JSON.stringify({
            message: 'Success',
        }),
    };
}
