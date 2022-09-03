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

    def run(input: str, rotate: int, flip: bool) -> str:
        w, h = map(int, input.splitlines()[0].split())
        block_count = int(input.splitlines()[1 + 4 * h])
        rotated_input = main_plaintext(input, rotate, flip)
        result = subprocess.run(args.solver, shell=True, input=rotated_input, text=True, capture_output=True)
        rotated_output = result.stdout
        rotated_width = h if rotate % 2 else w
        rotated_height = w if rotate % 2 else h
        flipped_output = main_isl(rotated_output, rotate, False, rotated_width, rotated_height, block_count)
        output = main_isl(flipped_output, 0, flip, w, h, block_count)
        return output

    if args.input_dir:
        if not args.output_dir:
            raise ValueError("output-dir is required when input-dir is specified")
        parent_dir = args.output_dir.parent
        for rotate in range(4):
            for flip in [False, True]:
                dir_name = f"{args.output_dir.name}-{rotate}-{str(flip).lower()}"
                output_dir = parent_dir / dir_name
                output_dir.mkdir(parents=True, exist_ok=True)
                for input_path in args.input_dir.iterdir():
                    with input_path.open() as input_file:
                        problem_id = input_path.name.replace(".txt", "")
                        if problem_id.startswith("ex") or int(problem_id) <= 25:
                            continue
                        output_path = output_dir / f"{problem_id}.isl"
                        print(output_path)
                        if output_path.exists():
                            continue
                        output_path.touch()
                        output = run(input_file.read(), rotate, flip)
                        with output_path.open('w') as output_file:
                            output_file.write(output)
    else:
        input = sys.stdin.read()
        sys.stdout.write(run(input, args.rotate, args.flip))