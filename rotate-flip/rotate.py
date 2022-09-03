from argparse import ArgumentParser
import sys
from typing import List
from io import StringIO


def _read_int_list(line: str) -> List[int]:
    return [int(x) for x in line.split()]


def _read_int_matrix(lines: List[str]) -> List[List[int]]:
    return [_read_int_list(line) for line in lines]


def main_plaintext(input: str, rotate: int, flip: bool) -> str:
    rotate = rotate % 4
    input_lines = input.splitlines()
    w, h = _read_int_list(input_lines[0])
    channels = [_read_int_matrix(input_lines[1 + h * channel: 1 + h * (channel + 1)]) for channel in range(4)]

    if flip:
        for channel in channels:
            for row in channel:
                row.reverse()

    for _ in range(rotate):
        h, w = w, h
        channels = [list(zip(*reversed(channel))) for channel in channels]
    
    buffer = StringIO()
    print(w, h, file=buffer)
    for channel in channels:
        for row in channel:
            print(' '.join(map(str, row)), file=buffer)
    return buffer.getvalue()
    

def main_isl(input: str, rotate: int, flip: bool) -> str:
    rotate = rotate % 4


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("--input-type", help="one of `plaintext` (competitive programming format input), `isl` (output)")
    parser.add_argument("--flip", action='store_true', help='reverse all *ROWS* *BEFORE* rotation')
    parser.add_argument("--rotate", type=int, help="rotate degree. 1 = 90 deg = pi / 2 rad. positive = ccw in mathematical coordinate. accepts negative values", default=0)
    args = parser.parse_args()

    if args.input_type == 'plaintext':
        input = sys.stdin.read()
        output = main_plaintext(input, args.rotate, args.flip)
        sys.stdout.write(output)
    elif args.input_type == 'isl':
        raise Exception("not implemented")
        input = sys.stdin.read()
        output = main_isl(input, args.rotate, args.flip)
        sys.stdout.write(output)
    else:
        raise ValueError(f"invalid input type: {args.input_type}")