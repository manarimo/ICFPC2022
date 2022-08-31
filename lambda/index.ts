import { APIGatewayEvent, APIGatewayProxyResult, Context } from 'aws-lambda';
import { S3 } from 'aws-sdk';
import * as fsPromises from 'fs/promises';
import { GetObjectOutput } from 'aws-sdk/clients/s3';
import { spawn } from 'child_process';

interface Args {
    executablePath: string;
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

function runProcess(path: string): Promise<string> {
    const proc = spawn(path);
    let buffer = '';
    proc.stdout.on('data', (data) => {
        buffer += data.toString();
    });

    return new Promise<string>((resolve, reject) => {
        proc.on('exit', () => resolve(buffer));
    });
}

export async function handler(event: Args, context: Context): Promise<APIGatewayProxyResult> {
    console.log(`Event: ${JSON.stringify(event, null, 2)}`);

    const fileHandle = await fsPromises.open('/tmp/a.out', 'w', 0o755);
    const obj = await getObject(event.executablePath);
    if (obj.Body == null) {
        return {
            statusCode: 400,
            body: 'Failed to download executable',
        };
    }

    await fileHandle.write(obj.Body as Buffer);
    await fileHandle.close();

    const output = await runProcess('/tmp/a.out');

    return {
        statusCode: 200,
        body: JSON.stringify({
            message: 'Success',
            output: output,
        }),
    };
}
