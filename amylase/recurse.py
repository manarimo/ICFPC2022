from pathlib import Path
from collections import defaultdict
import itertools


root_dir = Path(__file__).parent.parent
problem_dir = root_dir / "problem" / "plaintext"


def color_distance(c1, c2) -> float:
    cost = 0
    for x1, x2 in zip(c1, c2):
        cost += (x1 - x2) ** 2
    return cost ** 0.5


def parse_file(problem_path):
    with problem_path.open() as problem_file:
        lines = problem_file.readlines()
        w, h = [int(x) for x in lines[0].split()]
        image = []
        for y, (rline, gline, bline, aline) in enumerate(zip(lines[1: 1 + h], lines[1 + h: 1 + 2 * h], lines[1 + 2 * h: 1 + 3 * h], lines[1 + 3 * h: ])):
            row = []
            rvs = [int(x) for x in rline.split()]
            gvs = [int(x) for x in gline.split()]
            bvs = [int(x) for x in bline.split()]
            avs = [int(x) for x in aline.split()]
            for x, (r, g, b, a) in enumerate(zip(rvs, gvs, bvs, avs)):
                color = r, g, b, a
                row.append(color)
            image.append(row)
        image.reverse()
        return image


def calculate_unicolor_cost_monochrome(image, top, bottom, left, right):
    w, h = len(image[0]), len(image)
    command_cost = round(5 * (w * h) / ((bottom - top) * (right - left)))
    color_counts = defaultdict(int)
    for y in range(top, bottom):
        for x in range(left, right):
            color_counts[image[y][x]] += 1
    def color_cost_func(color):
        return sum(color_distance(image_color, color) * count for image_color, count in color_counts.items())
    color_sums = sum([[sum(color)] * count for color, count in color_counts.items()], [])
    color_sums.sort()
    best_color_value = round((color_sums[len(color_sums) // 2] - 255) // 3)
    best_color = best_color_value, best_color_value, best_color_value, 255
    best_cost = color_cost_func(best_color)
    return best_cost * 0.005 + command_cost, best_color


def calculate_unicolor_cost(image, top, bottom, left, right):
    # returns: float, color
    precision = 4

    w, h = len(image[0]), len(image)
    command_cost = round(5 * (w * h) / ((bottom - top) * (right - left)))
    color_counts = defaultdict(int)
    for y in range(top, bottom):
        for x in range(left, right):
            color_counts[image[y][x]] += 1
    def color_cost_func(color):
        return sum(color_distance(image_color, color) * count for image_color, count in color_counts.items())
    reds, greens, blues, alphas = zip(*color_counts.keys())
    best_cost, best_color = color_cost_func((0, 0, 0, 255)), (0, 0, 0, 255)
    for r, g, b in itertools.product(range(min(reds), max(reds) + 1, precision), range(min(greens), max(greens) + 1, precision), range(min(blues), max(blues) + 1, precision), ):
        candidate_color = r, g, b, 255
        candidate_cost = color_cost_func(candidate_color)
        if candidate_cost < best_cost:
            best_cost = candidate_cost
            best_color = candidate_color
    return best_cost * 0.005 + command_cost, best_color


def color_command(block_id, color):
    color_str = ','.join(map(str, color))
    return f"color [{block_id}] [{color_str}]"


def solve(image, top, bottom, left, right, block_id, cost_upperbound=1e100):
    size_precision = 2
    w, h = len(image[0]), len(image)
    area = (bottom - top) * (right - left)
    unicolor_cost, best_color = calculate_unicolor_cost_monochrome(image, top, bottom, left, right)
    if bottom - top < size_precision or right - left < size_precision or unicolor_cost > cost_upperbound:
        return unicolor_cost, [color_command(block_id, best_color)]

    ymid = (top + bottom) // 2
    xmid = (left + right) // 2
    division_cost = 10 * round(w * h / area)
    sub_commands = [f"cut [{block_id}] [{xmid},{ymid}]"]
    if division_cost >= unicolor_cost:
        return unicolor_cost, [color_command(block_id, best_color)]

    for sub_id in range(4):
        new_id = f"{block_id}.{sub_id}"
        xbig = sub_id in [1, 2]
        ybig = sub_id in [2, 3]
        ntop = ymid if ybig else top
        nbottom = bottom if ybig else ymid
        nleft = xmid if xbig else left
        nright = right if xbig else xmid
        sub_cost, sub_command = solve(image, ntop, nbottom, nleft, nright, new_id, unicolor_cost - division_cost)
        division_cost += sub_cost
        sub_commands += sub_command
        if division_cost >= unicolor_cost:
            return unicolor_cost, [color_command(block_id, best_color)]
    return division_cost, sub_commands


for problem_path in problem_dir.iterdir():
    print(problem_path)
    problem_id = problem_path.name.replace(".txt", "")
    image = parse_file(problem_path)
    cost, commands = solve(image, 0, len(image), 0, len(image[0]), "0")
    print(f"cost: {cost}, {len(commands)} lines")
    with (root_dir / "output" / "amylase-recursive-monochrome" / f"{problem_id}.isl").open("w") as f:
        f.write('\n'.join(commands))