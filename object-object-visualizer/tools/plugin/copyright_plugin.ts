import { Input, Output, Processor } from '../metaprocessor';
import { Move } from '../../src/parser';

export class CopyrightPlugin implements Processor {
    async run(input: Input, next: (input: Input) => Promise<Output>): Promise<Output> {
        const output = await next(input);
        const moves: Move[] = [{ kind: 'comment-move', comment: '(C) 2022 team manarimo. All rights reserved' }, ...output.moves];
        return new Output(moves);
    }
}
