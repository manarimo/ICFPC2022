from argparse import ArgumentParser
import subprocess
import sys
from rotate import main_isl, main_plaintext


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("--solver", required=True, help="command to execute the solver")
    parser.add_argument("--flip", action='store_true', help='reverse all *ROWS* *BEFORE* rotation')
    parser.add_argument("--rotate", type=int, help="rotate degree. 1 = 90 deg = pi / 2 rad. positive = ccw in mathematical coordinate. accepts negative values", default=0)
    args = parser.parse_args()

    input = sys.stdin.read()
    w, h = map(int, input.splitlines()[0].split())

    rotated_input = main_plaintext(input, args.rotate, args.flip)
    result = subprocess.run(args.solver, shell=True, input=rotated_input, text=True, capture_output=True)
    rotated_output = result.stdout
    if args.rotate % 1 == 1:
        w, h = h, w
    flipped_output = main_isl(rotated_output, args.rotate, False, w, h)
    if args.rotate % 1 == 1:
        w, h = h, w
    output = main_isl(flipped_output, 0, args.flip, w, h)
    sys.stdout.write(output)