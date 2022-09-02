from PIL import Image
from pathlib import Path


root_dir = Path(__file__).parent.parent
problem_dir = root_dir / "problem" / "original"

for problem_path in problem_dir.iterdir():
    if not problem_path.name.endswith(".png"):
        continue
    print(f"{problem_path}")
    problem_id = problem_path.name.replace(".png", "")
    plaintext_path = root_dir / "problem" / "plaintext" / (problem_id + ".txt")

    with Image.open(problem_path) as image:
        _, _, w, h = image.getbbox()

        with open(plaintext_path, "w") as target_file:
            print(f"{w} {h}", file=target_file)
            for band_id in range(4):
                pixels = list(image.getdata(band_id))
                for r in range(h):
                    offset = r * w
                    print(' '.join(map(str, pixels[offset: offset + w])), file=target_file)

        for band_id, band_name in enumerate(image.getbands()):
            band_image = Image.new('L', (w, h))
            band_image_file = root_dir / "problem" / "channels" / f"{problem_id}.{band_name}.png"
            pixels = list(image.getdata(band_id))
            for r in range(h):
                for c in range(w):
                    band_image.putpixel((c, r), pixels[r * w + c])
            print(band_image)
            band_image.save(band_image_file, "png")

