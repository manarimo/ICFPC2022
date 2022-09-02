from PIL import Image
import sys

if __name__ == '__main__':
    targets = sys.argv[1:]

    for target in targets:
        print(f"converting: {target}")
        target_file = target + ".txt"
        with Image.open(target) as image, open(target_file, "w") as target_file:

            print(f"bands: {image.getbands()}")
            print(f"bounding box: {image.getbbox()}")
            _, _, w, h = image.getbbox()
            print(f"{w} {h}", file=target_file)
            for band_id in range(4):
                pixels = list(image.getdata(band_id))
                for r in range(h):
                    offset = r * w
                    print(' '.join(map(str, pixels[offset: offset + w])), file=target_file)