import argparse
import os
import re
import shutil
import struct
import sys

from PIL import Image


OUTPUT_DIR_NAME = "output"
RESIZED_DIR_NAME = "resized"
QUALITY_DIR_NAME = "quality"
COLLAGE_DIR_NAME = "collage"
OUTPUT_BIN_NAME = "output.bin"
TARGET_SIZE = (320, 240)
COLLAGE_CELL_SIZE = (64, 48)
COLLAGE_GRID = (5, 5)
SUPPORTED_IMAGE_EXTENSIONS = {".jpg", ".jpeg", ".png", ".webp", ".avif"}


def is_supported_image_path(path: str) -> bool:
    _, ext = os.path.splitext(path)
    return ext.lower() in SUPPORTED_IMAGE_EXTENSIONS


def natural_sort_key(filename: str):
    """Convert a filename to a natural sort key for numeric ordering."""
    return [int(text) if text.isdigit() else text.lower() for text in re.split(r'(\d+)', filename)]



def ensure_output_dir(base_dir: str) -> str:
    output_dir = os.path.join(base_dir, OUTPUT_DIR_NAME)
    os.makedirs(output_dir, exist_ok=True)
    return output_dir


def normalize_image(image: Image.Image) -> Image.Image:
    if image.mode != "RGB":
        return image.convert("RGB")
    return image


def resize_to_target(image: Image.Image, path_hint: str) -> Image.Image:
    if image.size != TARGET_SIZE:
        return image.resize(TARGET_SIZE, Image.LANCZOS)
    return image


def collect_images_from_directory(directory: str) -> list[str]:
    if not os.path.isdir(directory):
        raise FileNotFoundError(f"Directory not found: {directory}")

    image_paths = []
    for filename in sorted(os.listdir(directory), key=natural_sort_key):
        if filename.startswith("."):
            continue
        full_path = os.path.join(directory, filename)
        if not os.path.isfile(full_path):
            continue
        _, ext = os.path.splitext(filename)
        if ext.lower() in SUPPORTED_IMAGE_EXTENSIONS:
            image_paths.append(full_path)
        else:
            print(f"{full_path} not supported")
    return image_paths


def save_intermediate_image(image: Image.Image, output_dir: str, filename: str) -> str:
    output_path = os.path.join(output_dir, filename)
    image.save(output_path)
    return output_path


def build_collage(images: list[Image.Image]) -> Image.Image:
    collage = Image.new("RGB", TARGET_SIZE)
    cell_w, cell_h = COLLAGE_CELL_SIZE
    max_cells = COLLAGE_GRID[0] * COLLAGE_GRID[1]
    for index in range(max_cells):
        x = (index % COLLAGE_GRID[0]) * cell_w
        y = (index // COLLAGE_GRID[0]) * cell_h
        if index < len(images):
            cell_image = images[index].resize(COLLAGE_CELL_SIZE, Image.LANCZOS)
            collage.paste(cell_image, (x, y))
        else:
            collage.paste(Image.new("RGB", COLLAGE_CELL_SIZE, (0, 0, 0)), (x, y))
    return collage


def encode_images_to_bin(jpeg_data_list: list[bytes], collage_count: int) -> bytes:
    total_images = len(jpeg_data_list)
    if total_images == 0:
        raise ValueError("At least one input image is required.")

    header_size = 8 + 4 * total_images
    offsets = []
    current_offset = header_size
    for data in jpeg_data_list:
        offsets.append(current_offset)
        current_offset += len(data)

    result = bytearray()
    result.extend(struct.pack("<I", collage_count))
    result.extend(struct.pack("<I", total_images))
    for offset in offsets:
        result.extend(struct.pack("<I", offset))
    for data in jpeg_data_list:
        result.extend(data)
    return bytes(result)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert multiple images to a single output.bin with JPEG data and a collage image.")
    parser.add_argument(
        "-q",
        dest="quality",
        type=int,
        default=80,
        help="JPEG quality for each converted image (1-95). Default is 80.",
    )
    parser.add_argument(
        "-a",
        "--all",
        dest="input_dir",
        help="Use all supported images from this input folder.",
        default=None,
    )
    parser.add_argument(
        "-c",
        "--clean",
        dest="clean",
        action="store_true",
        help="Clean the output folder before processing.",
    )
    parser.add_argument(
        "images",
        nargs="*",
        help="One or more input image paths.",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if not 1 <= args.quality <= 95:
        print("Error: JPEG quality must be between 1 and 95.", file=sys.stderr)
        sys.exit(1)

    cwd = os.getcwd()
    output_dir = ensure_output_dir(cwd)
    output_bin_path = os.path.join(cwd, OUTPUT_BIN_NAME)
    if os.path.isfile(output_bin_path):
        os.remove(output_bin_path)

    if args.clean:
        shutil.rmtree(output_dir)
        os.makedirs(output_dir, exist_ok=True)

    resized_dir = os.path.join(output_dir, RESIZED_DIR_NAME)
    quality_dir = os.path.join(output_dir, QUALITY_DIR_NAME)
    collage_dir = os.path.join(output_dir, COLLAGE_DIR_NAME)
    collage_resize_dir = os.path.join(collage_dir, RESIZED_DIR_NAME)
    collage_quality_dir = os.path.join(collage_dir, QUALITY_DIR_NAME)
    os.makedirs(resized_dir, exist_ok=True)
    os.makedirs(quality_dir, exist_ok=True)
    os.makedirs(collage_resize_dir, exist_ok=True)
    os.makedirs(collage_quality_dir, exist_ok=True)

    image_paths: list[str] = []
    if args.input_dir:
        try:
            image_paths.extend(collect_images_from_directory(args.input_dir))
        except FileNotFoundError as error:
            print(f"Error: {error}", file=sys.stderr)
            sys.exit(1)

    for image_path in args.images:
        if not os.path.isfile(image_path):
            print(f"Error: file not found: {image_path}", file=sys.stderr)
            sys.exit(1)
        if not is_supported_image_path(image_path):
            print(f"{image_path} not supported")
            continue
        image_paths.append(image_path)

    if not image_paths:
        print("Error: at least one supported image path or an -a folder is required.", file=sys.stderr)
        sys.exit(1)

    total_input_images = len(image_paths)
    print(f"Processing {total_input_images} images.")

    processed_images: list[Image.Image] = []
    jpeg_bytes_list: list[bytes] = []

    for index, image_path in enumerate(image_paths):
        if not os.path.isfile(image_path):
            print(f"Error: file not found: {image_path}", file=sys.stderr)
            sys.exit(1)

        resized_name = f"resized_{index}_{os.path.basename(image_path)}"
        quality_filename = f"quality_{index}_{args.quality}_{os.path.basename(image_path)}"
        resized_path = os.path.join(resized_dir, resized_name)
        quality_path = os.path.join(quality_dir, quality_filename)

        resized_exists = os.path.isfile(resized_path)
        quality_exists = os.path.isfile(quality_path)

        if resized_exists and quality_exists and not args.clean:
            with Image.open(resized_path) as cached_img:
                processed_images.append(cached_img.copy())
            with open(quality_path, "rb") as jpeg_file:
                jpeg_bytes_list.append(jpeg_file.read())
        else:
            with Image.open(image_path) as img:
                img = normalize_image(img)
                img = resize_to_target(img, image_path)
                save_intermediate_image(img, resized_dir, resized_name)

                img.save(quality_path, format="JPEG", quality=args.quality)
                with open(quality_path, "rb") as jpeg_file:
                    jpeg_bytes = jpeg_file.read()
                jpeg_bytes_list.append(jpeg_bytes)
                processed_images.append(img)

    collage_count = (len(processed_images) + 24) // 25
    collage_jpeg_list = []
    for collage_index in range(collage_count):
        collage_images = processed_images[collage_index * 25 : (collage_index + 1) * 25]
        collage = build_collage(collage_images)
        save_intermediate_image(collage, collage_resize_dir, f"collage_{collage_index}.png")
        collage_jpeg_path = os.path.join(collage_quality_dir, f"collage_{collage_index}_{args.quality}.jpg")
        collage.save(collage_jpeg_path, format="JPEG", quality=args.quality)
        
        # Read and store collage JPEG data
        with open(collage_jpeg_path, "rb") as collage_jpeg_file:
            collage_jpeg_list.append(collage_jpeg_file.read())

    # Combine: collages first, then individual images
    final_jpeg_bytes_list = collage_jpeg_list + jpeg_bytes_list

    with open(output_bin_path, "wb") as output_bin_file:
        output_bin_file.write(encode_images_to_bin(final_jpeg_bytes_list, collage_count))

    print(f"Created: {output_bin_path}")
    print(f"Intermediate files are in: {output_dir}")


if __name__ == "__main__":
    main()
