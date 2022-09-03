from argparse import ArgumentParser
import subprocess
import sys
from rotate import main_isl, main_plaintext
from pathlib import Path


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("--input-dir", type=Path)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--solver", required=True, help="command to execute the solver")
    parser.add_argument("--flip", action='store_true', help='reverse all *ROWS* *BEFORE* rotation')
    parser.add_argument("--rotate", type=int, help="rotate degree. 1 = 90 deg = pi / 2 rad. positive = ccw in mathematical coordinate. accepts negative values", default=0)
    args = parser.parse_args()

    def run(input: str) -> str:
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
        return output

    if args.input_dir:
        if not args.output_dir:
            raise ValueError("output-dir is required when input-dir is specified")
        parent_dir = args.output_dir.parent
        for rotate in range(4):
            for flip in [False, True]:
                dir_name = f"{args.output_dir.name}-{rotate}-{str(flip).lower()}"
                output_dir = parent_dir / dir_name
                output_dir.mkdir(parents=True)
                for input_path in args.input_dir.iterdir():
                    with input_path.open() as input_file:
                        problem_id = input_path.name.replace(".txt", "")
                        output_path = output_dir / f"{problem_id}.isl"
                        print(output_path)
                        output = run(input_file.read())
                        with output_path.open('w') as output_file:
                            output_file.write(output)
    else:
        input = sys.stdin.read()
        sys.stdout.write(run(sys.stdin.read()))