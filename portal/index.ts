import serverlessExpress from '@vendia/serverless-express';
import express from 'express';
import { S3 } from 'aws-sdk';
import { GetObjectOutput, ListObjectsV2Request } from 'aws-sdk/clients/s3';
import cors from 'cors';

const app = express();
app.use(cors());
const s3 = new S3();

function listDirectories(prefix: string): Promise<string[]> {
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
                const list = data.CommonPrefixes!!.map((c) => c.Prefix!!);
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
    const list = rawList.map((item) => item.substring(7, item.length - 1));
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

export const handler = serverlessExpress({ app });
