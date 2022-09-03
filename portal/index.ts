import serverlessExpress from '@vendia/serverless-express';
import express from 'express';
import { S3 } from 'aws-sdk';
import { GetObjectOutput, ListObjectsV2Request } from 'aws-sdk/clients/s3';
import cors from 'cors';

class S3Item {
    constructor(readonly path: string) {}

    get baseName(): string {
        const match = this.path.match(/([^\/]+)(?:$|\/$)/);
        if (match == null) {
            throw new Error(`Fail to extract basename for ${this.path}`);
        }
        return match[1];
    }
}

class SolutionSpec {
    constructor(readonly batchName: string, readonly problemId: string) {}
}

const app = express();
app.use(cors());
const s3 = new S3();

function listDirectories(prefix: string): Promise<S3Item[]> {
    const req: ListObjectsV2Request = {
        Bucket: 'icfpc2022-manarimo',
        Prefix: prefix,
        Delimiter: '/',
    };
    return new Promise((resolve, reject) => {
        s3.listObjectsV2(req, (err, data) => {
            if (err) {
                console.log('ListObjectsV2 rejected', err);
                reject(err);
                return;
            }

            try {
                const list = data.CommonPrefixes!!.map((c) => new S3Item(c.Prefix!!));
                resolve(list);
            } catch (e) {
                reject(e);
            }
        });
    });
}

function listObjects(prefix: string): Promise<S3Item[]> {
        const req: ListObjectsV2Request = {
        Bucket: 'icfpc2022-manarimo',
        Prefix: prefix,
        Delimiter: '/',
    };
    return new Promise((resolve, reject) => {
        s3.listObjectsV2(req, (err, data) => {
            if (err) {
                console.log('ListObjectsV2 rejected', err);
                reject(err);
                return;
            }

            try {
                const list = data.Contents!!.map((c) => new S3Item(c.Key!!));
                resolve(list);
            } catch (e) {
                reject(e);
            }
        });
    });
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

app.get('/api/batches', async function (req, res) {
    const rawList = await listDirectories('output/');
    const list = rawList.map((item) => item.baseName);
    res.json({
        items: list,
    });
});

// GET /api/solution?problemId=1&batch=sample-test
app.get('/api/solution', async function (req, res) {
    const problemId = req.query['problemId'];
    const batch = req.query['batch'];
    const obj = await getObject(`output/${batch}/${problemId}.isl`);

    res.json({
        code: obj.Body!!.toString(),
    });
});

app.get('/api/solution_all', async function (req, res) {
    const batches = await listDirectories('output/');
    const solutions: Record<string, SolutionSpec[]> = {};
    const loading = batches.map(async (batch) => {
        const items = await listObjects(batch.path);
        solutions[batch.baseName] = items
            .filter((item) => item.path.endsWith('.isl'))
            .map((item) => new SolutionSpec(batch.baseName, item.baseName.slice(0, -4)))
            .sort((a, b) => parseInt(a.problemId) - parseInt(b.problemId));
    });
    await Promise.all(loading);

    const orderedSolutions = batches.map((b) => ({
        batchName: b.baseName,
        solutions: solutions[b.baseName],
    }));
    res.json({
        solutions: orderedSolutions,
    });
});

export const handler = serverlessExpress({ app });

app.listen(8080);