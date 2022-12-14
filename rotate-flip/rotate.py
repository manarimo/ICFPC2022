from argparse import ArgumentParser
import sys
from typing import List
from io import StringIO
import isl
from pathlib import Path


def _read_int_list(line: str) -> List[int]:
    return [int(x) for x in line.split()]


def _read_int_matrix(lines: List[str]) -> List[List[int]]:
    return [_read_int_list(line) for line in lines]


def _read_blocks(lines: List[str]):
    return {
        "blockId": lines[0].strip(),
        "bottomLeft": _read_int_list(lines[1]),
        "topRight": _read_int_list(lines[2]),
        "color": _read_int_list(lines[3]),
    }


def main_plaintext(input: str, rotate: int, flip: bool) -> str:
    rotate = rotate % 4
    input_lines = input.splitlines()
    w, h = _read_int_list(input_lines[0])
    channels = [_read_int_matrix(input_lines[1 + h * channel: 1 + h * (channel + 1)]) for channel in range(4)]
    block_count = int(input_lines[1 + h * 4])
    blocks = [_read_blocks(input_lines[1 + h * 4 + 1 + 4 * block_index: 1 + h * 4 + 1 + 4 * (block_index + 1)]) for block_index in range(block_count)]

    if flip:
        for channel in channels:
            for row in channel:
                row.reverse()
        for block in blocks:
            left, bottom = block["bottomLeft"]
            right, top = block["topRight"]
            block["bottomLeft"] = [bottom, w - right]
            block["topRight"] = [top, w - left]

    for _ in range(rotate):
        for block in blocks:
            left, bottom = block["bottomLeft"]
            right, top = block["topRight"]
            block["bottomLeft"] = [h - top, left]
            block["topRight"] = [h - bottom, right]
        channels = [list(zip(*reversed(channel))) for channel in channels]
        h, w = w, h
    
    buffer = StringIO()
    print(w, h, file=buffer)
    for channel in channels:
        for row in channel:
            print(' '.join(map(str, row)), file=buffer)
    print(len(blocks), file=buffer)
    for block in blocks:
        print(block["blockId"], file=buffer)
        print(" ".join(map(str, block["bottomLeft"])), file=buffer)
        print(" ".join(map(str, block["topRight"])), file=buffer)
        print(" ".join(map(str, block["color"])), file=buffer)
    return buffer.getvalue()
    

def main_isl(input: str, rotate: int, flip: bool, width: int, height: int, initial_blocks: int) -> str:
    rotate = rotate % 4
    commands = isl.parse(input)
    if flip:
        new_commands = []
        block_ids = {str(block_id): str(block_id) for block_id in range(initial_blocks)}  # old -> new
        global_counter = initial_blocks
        for command in commands:
            if command['move'] == 'pcut':
                original_id = command['block']
                mapped_id = block_ids[original_id]
                for sub_block in range(4):
                    block_ids[original_id + f".{sub_block}"] = mapped_id + f".{sub_block ^ 1}"
                px, py = command['point']
                new_commands.append({
                    'move': 'pcut',
                    'block': mapped_id,
                    'point': (width - px, py),
                })
            elif command['move'] == 'lcut':
                original_id = command['block']
                mapped_id = block_ids[original_id]
                isX = command['orientation'] == 'x'
                for sub_block in range(2):
                    if isX:
                        block_ids[original_id + f".{sub_block}"] = mapped_id + f".{sub_block ^ 1}"
                    else:
                        block_ids[original_id + f".{sub_block}"] = mapped_id + f".{sub_block}"
                new_commands.append({
                    'move': 'lcut',
                    'block': block_ids[command['block']],
                    'orientation': command['orientation'],
                    'line-number': (width - command['line-number']) if isX else command['line-number'],
                })
            elif command['move'] == 'color':
                new_commands.append({
                    'move': 'color',
                    'block': block_ids[command['block']],
                    'color': command['color'],
                })
            elif command['move'] == 'swap':
                new_commands.append({
                    'move': 'swap',
                    'block': block_ids[command['block']],
                    'target': block_ids[command['target']],
                })
            elif command['move'] == 'merge':
                new_block_id = str(global_counter)
                global_counter += 1
                block_ids[new_block_id] = new_block_id
                new_commands.append({
                    'move': 'merge',
                    'block': block_ids[command['block']],
                    'target': block_ids[command['target']],
                })
            else:
                raise ValueError(f"unknown command move: {command['move']}")
        commands = new_commands

    for _ in range(rotate):
        new_height, new_width = width, height
        new_commands = []
        block_ids = {str(block_id): str(block_id) for block_id in range(initial_blocks)}  # old -> new
        global_counter = initial_blocks
        for command in commands:
            if command['move'] == 'pcut':
                original_id = command['block']
                mapped_id = block_ids[original_id]
                for sub_block in range(4):
                    block_ids[original_id + f".{sub_block}"] = mapped_id + f".{(sub_block + 1) % 4}"
                px, py = command['point']
                new_commands.append({
                    'move': 'pcut',
                    'block': mapped_id,
                    'point': (height - py, px),
                })
            elif command['move'] == 'lcut':
                original_id = command['block']
                mapped_id = block_ids[original_id]
                isX = command['orientation'] == 'x'
                for sub_block in range(2):
                    if isX:
                        block_ids[original_id + f".{sub_block}"] = mapped_id + f".{sub_block}"
                    else:
                        block_ids[original_id + f".{sub_block}"] = mapped_id + f".{sub_block ^ 1}"
                new_commands.append({
                    'move': 'lcut',
                    'block': block_ids[command['block']],
                    'orientation': 'y' if isX else 'x',
                    'line-number': command['line-number'] if isX else (height - command['line-number']),
                })
            elif command['move'] == 'color':
                new_commands.append({
                    'move': 'color',
                    'block': block_ids[command['block']],
                    'color': command['color'],
                })
            elif command['move'] == 'swap':
                new_commands.append({
                    'move': 'swap',
                    'block': block_ids[command['block']],
                    'target': block_ids[command['target']],
                })
            elif command['move'] == 'merge':
                new_block_id = str(global_counter)
                global_counter += 1
                block_ids[new_block_id] = new_block_id
                new_commands.append({
                    'move': 'merge',
                    'block': block_ids[command['block']],
                    'target': block_ids[command['target']],
                })
            else:
                raise ValueError(f"unknown command move: {command['move']}")
        commands = new_commands
        height, width = new_height, new_width

    return isl.serialize(commands)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("--input-type", help="one of `plaintext` (competitive programming format input), `isl` (output)")
    parser.add_argument("--flip", action='store_true', help='reverse all *ROWS* *BEFORE* rotation')
    parser.add_argument("--rotate", type=int, help="rotate degree. 1 = 90 deg = pi / 2 rad. positive = ccw in mathematical coordinate. accepts negative values", default=0)
    parser.add_argument("--problem", type=Path, help="path to plaintext problem. used in isl mode. if skipped, lightning definition will be used.")
    args = parser.parse_args()

    if args.input_type == 'plaintext':
        input = sys.stdin.read()
        output = main_plaintext(input, args.rotate, args.flip)
        sys.stdout.write(output)
    elif args.input_type == 'isl':
        if args.problem is None:
            width, height = 400, 400
            initial_blocks = 1
        else:
            with args.problem.open() as f:
                lines = f.readlines()
                width, height = map(int, lines[0].split())
                initial_blocks = int(lines[1 + 4 * height])
        input = sys.stdin.read()
        output = main_isl(input, args.rotate, args.flip, width, height, initial_blocks)
        sys.stdout.write(output)
    else:
        raise ValueError(f"invalid input type: {args.input_type}")