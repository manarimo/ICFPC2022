import { topNSolutions } from './util';
import * as fs from 'fs';
import * as https from 'https';
import FormData from 'form-data';

async function main() {
    for await (let best of topNSolutions(1)) {
        const solutionPath = `../../output/${best.batchName}/${best.problemId}.isl`;
        const stream = fs.createReadStream(solutionPath);
        const form = new FormData();
        form.append('file', stream);

        const options: https.RequestOptions = {
            hostname: 'robovinci.xyz',
            path: `/api/submissions/${best.problemId}/create`,
            method: 'POST',
            headers: {
                Authorization: `Bearer ${process.argv[2]}`,
                ...form.getHeaders(),
            },
        };
        console.log(`Submitting problem ${best.problemId}`);

        const promise = new Promise<void>((resolve, reject) => {
            const req = https.request(options, (res) => {
                res.on('data', (chunk) => {
                    console.log(chunk.toString());
                });
                res.on('end', () => resolve());
            });
            req.on('error', (e) => reject(e));
            form.pipe(req);
        });
        await promise;
    }
}

if (process.argv.length < 3) {
    console.error('Usage: npm run send-best -- (api key)');
    process.exit(1);
}

main();
