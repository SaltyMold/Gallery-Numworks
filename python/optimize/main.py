from __future__ import annotations

import argparse
from pathlib import Path
from PIL import Image

SUPPORTED_EXTENSIONS = {'.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tiff', '.webp'}
TARGET_WIDTH = 1280
TARGET_HEIGHT = 720
TARGET_SIZE = (TARGET_WIDTH, TARGET_HEIGHT)


def center_crop_and_resize(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    target_width, target_height = size
    source_width, source_height = image.size

    # Scale image to at least the target size while keeping aspect ratio.
    scale = max(target_width / source_width, target_height / source_height)
    new_width = int(source_width * scale)
    new_height = int(source_height * scale)
    image = image.resize((new_width, new_height), Image.LANCZOS)

    left = (new_width - target_width) // 2
    top = (new_height - target_height) // 2
    right = left + target_width
    bottom = top + target_height

    return image.crop((left, top, right, bottom))


def convert_images(input_dir: Path, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)

    image_files = [path for path in sorted(input_dir.iterdir()) if path.suffix.lower() in SUPPORTED_EXTENSIONS]
    if not image_files:
        print(f'No supported image files found in "{input_dir}".')
        return

    for source_path in image_files:
        try:
            with Image.open(source_path) as image:
                image = image.convert('RGB')
                image = center_crop_and_resize(image, TARGET_SIZE)

                output_name = source_path.stem + '.jpg'
                output_path = output_dir / output_name
                image.save(output_path, format='JPEG', quality=85, optimize=True)

                print(f'Converted: {source_path.name} -> {output_path.name}')
        except Exception as exc:
            print(f'Failed to convert "{source_path.name}": {exc}')


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Convert images from input folder to 720p 16:9 JPEG files.')
    parser.add_argument('--input', '-i', type=Path, default=Path('input'), help='Input folder containing image files.')
    parser.add_argument('--output', '-o', type=Path, default=Path('output'), help='Output folder for converted JPEG files.')
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    convert_images(args.input, args.output)


if __name__ == '__main__':
    main()
