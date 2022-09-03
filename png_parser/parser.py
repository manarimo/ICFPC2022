import json
from PIL import Image
from pathlib import Path


root_dir = Path(__file__).parent.parent
lightning_initial = {
    "width": 400,
    "height": 400,
    "blocks": [{
        "blockId": "0",
        "bottomLeft": [0, 0],
        "topRight": [400, 400],
        "color": [255, 255, 255, 255],
    }],
}


def process_dir(problem_dir, suffix=""):
    for problem_path in problem_dir.iterdir():
        if not problem_path.name.endswith(".png"):
            continue
        print(f"{problem_path}")
        problem_id = problem_path.name.replace(".png", "")
        plaintext_path = root_dir / "problem" / f"plaintext{suffix}" / (problem_id + ".txt")
        initial_path = problem_dir / f"{problem_id}.initial.json"

        with open(plaintext_path, "w") as target_file:
            with Image.open(problem_path) as image:
                _, _, w, h = image.getbbox()

                print(f"{w} {h}", file=target_file)
                for band_id in range(4):
                    pixels = list(image.getdata(band_id))
                    for r in range(h):
                        offset = r * w
                        print(' '.join(map(str, pixels[offset: offset + w])), file=target_file)

                for band_id, band_name in enumerate(image.getbands()):
                    band_image = Image.new('L', (w, h))
                    band_image_file = root_dir / "problem" / f"channels{suffix}" / f"{problem_id}.{band_name}.png"
                    pixels = list(image.getdata(band_id))
                    for r in range(h):
                        for c in range(w):
                            band_image.putpixel((c, r), pixels[r * w + c])
                    band_image.save(band_image_file, "png")
            
            if initial_path.exists():
                with initial_path.open() as initial_file:
                    initial_config = json.load(initial_file)
            else:
                initial_config = lightning_initial
            print(len(initial_config["blocks"]), file=target_file)
            for block in initial_config["blocks"]:
                print(block["blockId"], file=target_file)
                print(' '.join(map(str, block["bottomLeft"])), file=target_file)
                print(' '.join(map(str, block["topRight"])), file=target_file)
                print(' '.join(map(str, block["color"])), file=target_file)


process_dir(root_dir / "problem" / "original")
process_dir(root_dir / "problem" / "color_reduced", "_color_reduced")