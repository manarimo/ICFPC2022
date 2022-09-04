import {InitialBlock} from "../../src/simulate";
import {Move} from "../../src/parser";
import {Image} from "../util";

export class Input {
    constructor(readonly image: Image, readonly initialBlocks: InitialBlock[]) {}
}

export class Output {
    constructor(readonly moves: Move[]) {}
}

export interface Processor {
    run(input: Input, next: (input: Input) => Promise<Output>): Promise<Output>
}
