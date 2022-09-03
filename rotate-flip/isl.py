from typing import List, Dict, Tuple
import re
from io import StringIO


def _parse_str(component: str) -> str:
    return component[1: -1]


def _parse_point(component: str) -> Tuple[int, int]:
    return tuple(int(x) for x in _parse_str(component).split(','))


def _parse_line_number(component: str) -> int:
    return int(_parse_str(component))


LINE_PATTERN = re.compile(r"([a-z]+?)(\[.+?\])(\[.+?\])?(\[.+?\])?")
def parse(raw_file: str) -> List[Dict]:
    commands = []
    for line in raw_file.splitlines():
        line = line.strip()
        if line.startswith('#'):
            continue
        line = line.replace(" ", "")
        match = LINE_PATTERN.search(line)
        if not match:
            raise ValueError("failed to parse command line: " + line)
        components = match.groups()
        move = components[0]
        if move == 'cut':
            if components[3] == None:
                commands.append({
                    "move": "pcut",
                    "block": _parse_str(components[1]),
                    "point": _parse_point(components[2]),
                })
            else:
                commands.append({
                    "move": "lcut",
                    "block": _parse_str(components[1]),
                    "orientation": _parse_str(components[2]).lower(),
                    "line-number": _parse_line_number(components[3]),
                })
        elif move == 'color':
            commands.append({
                "move": move,
                "block": _parse_str(components[1]),
                "color": _parse_str(components[2]),
            })
        elif move == 'swap':
            commands.append({
                "move": move,
                "block": _parse_str(components[1]),
                "target": _parse_str(components[2]),
            })
        elif move == 'merge':
            commands.append({
                "move": move,
                "block": _parse_str(components[1]),
                "target": _parse_str(components[2]),
            })
        else:
            raise ValueError("unknown move type: " + move)
    return commands


def serialize(commands: List[Dict]) -> str:
    buffer = StringIO()
    for command in commands:
        if command['move'] == 'pcut':
            point_str = ','.join(map(str, command["point"]))
            print(f"cut [{command['block']}] [{point_str}]", file=buffer)
        elif command['move'] == 'lcut':
            print(f"cut [{command['block']}] [{command['orientation']}] [{command['line-number']}]", file=buffer)
        elif command['move'] == 'color':
            print(f"color [{command['block']}] [{command['color']}]", file=buffer)
        elif command['move'] == 'swap':
            print(f"swap [{command['block']}] [{command['target']}]", file=buffer)
        elif command['move'] == 'merge':
            print(f"merge [{command['block']}] [{command['target']}]", file=buffer)
        else:
            print(f"ignoring unknown command move: {command['move']}")
    return buffer.getvalue()
