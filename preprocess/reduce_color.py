from pathlib import Path
from collections import defaultdict
from PIL import Image


root_dir = Path(__file__).parent.parent
problem_dir = root_dir / "problem" / "plaintext"

reduced_colors = {
    "1.txt": 3,
    "2.txt": 12,
    "3.txt": 12,
    "4.txt": 2,
    "5.txt": 2,
    "6.txt": 12,
    "7.txt": 5,
    "8.txt": 12,
    "9.txt": 10,
    "10.txt": 10,
}

def color_distance(c1, c2):
    cost = 0
    for x1, x2 in zip(c1, c2):
        cost += (x1 - x2) ** 2
    return cost ** 0.5


for problem_path in problem_dir.iterdir():
    print(problem_path)
    with problem_path.open() as problem_file:
        lines = problem_file.readlines()
        w, h = [int(x) for x in lines[0].split()]
        color_count = defaultdict(int)
        pixel_mapping = dict()
        for y, (rline, gline, bline, aline) in enumerate(zip(lines[1: 1 + h], lines[1 + h: 1 + 2 * h], lines[1 + 2 * h: 1 + 3 * h], lines[1 + 3 * h: ])):
            rvs = [int(x) for x in rline.split()]
            gvs = [int(x) for x in gline.split()]
            bvs = [int(x) for x in bline.split()]
            avs = [int(x) for x in aline.split()]
            for x, (r, g, b, a) in enumerate(zip(rvs, gvs, bvs, avs)):
                color = r, g, b, a
                color_count[color] += 1
                pixel_mapping[x, y] = color
        
        counts = list(color_count.values())
        counts.sort(reverse=True)
        reduced_color = reduced_colors.get(problem_path.name, 20)
        threshold = counts[reduced_color] if len(counts) > reduced_color else 0
        print(counts[:20])

        color_mapping = dict()
        use_colors = set()
        for color, count in color_count.items():
            if count > threshold:
                use_colors.add(color)
        for color, count in color_count.items():
            if count <= threshold:
                init_color = list(use_colors)[0]
                best_color, best_distance = init_color, color_distance(init_color, color)
                for use_color in use_colors:
                    distance = color_distance(use_color, color)
                    if distance < best_distance:
                        best_color, best_distance = color, distance
                color_mapping[color] = best_color
        image = Image.new('RGBA', (w, h))
        for x in range(w):
            for y in range(h):
                original_color = pixel_mapping[x, y]
                if original_color in use_colors:
                    image.putpixel((x, y), original_color)
                else:
                    image.putpixel((x, y), color_mapping[original_color])
        image.save(root_dir / "problem" / "color_reduced" / problem_path.name.replace("txt", "png"), "png")