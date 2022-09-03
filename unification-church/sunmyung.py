# unify and wipe out everything before run

from argparse import ArgumentParser
import io
import subprocess
import sys
import isl


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("--solver", required=True, help="command to execute the solver")
    parser.add_argument("--rewrite-id", action="store_true", help="rewrites block id in solver response assuming that the solver is for lightning division.")
    args = parser.parse_args()

    input = sys.stdin.read()
    input_lines = input.splitlines()
    w, h = map(int, input_lines[0].split())
    block_count = int(input_lines[1 + 4 * h])
    blocks = []
    for block_index in range(block_count):
        offset = 1 + 4 * h + 1 + 4 * block_index
        block_id = input_lines[offset]
        left, bottom = map(int, input_lines[offset + 1].split())
        right, top = map(int, input_lines[offset + 2].split())
        color = list(map(int, input_lines[offset + 3].split()))
        blocks.append({
            "id": block_id,
            "left": left,
            "bottom": bottom,
            "top": top,
            "right": right,
            "color": color
        })

    next_block_id = block_count

    blocks.sort(key=lambda block: (block["bottom"], block["left"]))
    # row merger
    first = 0
    while first < len(blocks):
        second = first + 1
        while second < len(blocks) and blocks[first]["right"] == blocks[second]["left"] and blocks[first]["bottom"] == blocks[second]["bottom"] and blocks[first]["top"] == blocks[second]["top"]:
            print(f"merge [{blocks[first]['id']}] [{blocks[second]['id']}]")
            new_block = {
                "id": str(next_block_id),
                "left": blocks[first]["left"],
                "bottom": blocks[first]["bottom"],
                "right": blocks[second]["right"],
                "top": blocks[second]["top"],
            }
            blocks = blocks[:first] + [new_block] + blocks[second + 1:]
            next_block_id += 1
        first += 1

    # col merger
    while len(blocks) > 1:
        first, second = blocks[:2]
        if first["top"] == second["bottom"] and first["left"] == second["left"] and first["right"] == second["right"]:
            print(f"merge [{first['id']}] [{second['id']}]")
            new_block = {
                "id": str(next_block_id),
                "left": first["left"],
                "bottom": first["bottom"],
                "right": second["right"],
                "top": second["top"],
            }
            blocks = [new_block] + blocks[2:]
            next_block_id += 1
        else:
            raise ValueError(f"merger step failed!!!")

    # coloring
    unified_block_id = block_count * 2 - 2
    print(f"color [{unified_block_id}] [255,255,255,255]")
    print(f"# powered by unification church")
    unified_input = io.StringIO()
    for line in input_lines[: 1 + 4 * h]:
        print(line, file=unified_input)
    print(unified_block_id, file=unified_input)
    print(0, 0, file=unified_input)
    print(w, h, file=unified_input)
    print(255, 255, 255, 255, file=unified_input)

    result = subprocess.run(args.solver, shell=True, input=unified_input.getvalue(), text=True, capture_output=True)
    solver_output = result.stdout

    if args.rewrite_id:
        commands = isl.parse(solver_output)
        def rewrite(block_id):
            if block_id.find(".") >= 0:
                base, sub = block_id.split(".", 1)
                return f"{int(base) + unified_block_id}.{sub}"
            else:
                return str(int(block_id) + unified_block_id)
        for command in commands:
            if command['move'] == 'pcut':
                command['block'] = rewrite(command['block'])
            elif command['move'] == 'lcut':
                command['block'] = rewrite(command['block'])
            elif command['move'] == 'color':
                command['block'] = rewrite(command['block'])
            elif command['move'] == 'swap':
                command['block'] = rewrite(command['block'])
                command['target'] = rewrite(command['target'])
            elif command['move'] == 'merge':
                command['block'] = rewrite(command['block'])
                command['target'] = rewrite(command['target'])
            else:
                raise ValueError("unknown move type: " + command["move"])
        solver_output = isl.serialize(commands)

    sys.stdout.write(solver_output)