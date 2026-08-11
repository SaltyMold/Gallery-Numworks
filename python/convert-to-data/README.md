# Image Conversion Utility

This script converts multiple input images into a single `output.bin` file. It also generates intermediate files inside an `output` folder.

## Requirements

- Python 3.8+
- Pillow (`pip install pillow`)

## Usage

Run the script from the `convert-to-data` directory or any working directory:

```bash
python main.py -q 80 img1.png img2.jpg ...
```

Or use `-a` to include all images from a folder:

```bash
python main.py -q 80 -a input/
```

You can also clean the output folder before processing with `-c`:

```bash
python main.py -q 80 -a input/ -c
```

- `-q 80` sets JPEG quality to 80.
- `-a input/` uses all supported images in the `input` folder.
- `-c` clears the `output` folder before processing and regenerates all images.
- Without `-c`, the script reuses existing images in `output` and only regenerates missing ones.
- Supported formats are: `.jpg`, `.jpeg`, `.png`, `.webp`, and `.avif`.
- Unsupported files are skipped and printed with `not supported`.
- You can pass as many image paths as you want (limited only by available memory).
- The script resizes every input image to `320x240` if needed.

## Output

- `output.bin` is written to the current working directory.
- All intermediate images are written to subfolders under the `output` directory.

### Intermediate folders

The `output` directory contains:

- `resized/`: each image converted to `320x240` and RGB.
- `quality/`: each converted image saved as JPEG at the requested quality.
- `collage/`: collage images.

### Intermediate files

The `output/resized` directory contains:

- `resized_<index>_<basename>`: each image converted to `320x240` and RGB.

The `output/quality` directory contains:

- `quality_<index>_<quality>_<basename>`: each converted image saved as JPEG at the requested quality.

The `output/collage` directory contains:

- `resize/`: PNG collage images.
- `quality/`: JPEG collage images.

The `output/collage/resize` directory contains:

- `collage_0.png`, `collage_1.png`, ...

The `output/collage/quality` directory contains:

- `collage_0_<quality>.jpg`, `collage_1_<quality>.jpg`, ...

## Binary format (`output.bin`)

The file format is:

1. Bytes 0..3: little-endian 32-bit number of collage images.
2. Bytes 4..7: little-endian 32-bit total number of input images.
3. Bytes 8..11: little-endian 32-bit offset of image 0.
4. Bytes 12..15: little-endian 32-bit offset of image 1 (if present).
5. Bytes 16..19: little-endian 32-bit offset of image 2 (if present).
6. ...
7. Image data sections: each image is stored as raw JPEG data starting at the corresponding offset.

- A collage image can contain up to 25 input images.

### Example layout for 2 images

- Bytes 0..3: number of collage images (32-bit)
- Bytes 4..7: total number of input images (32-bit)
- Bytes 8..11: offset for image 0 (32-bit)
- Bytes 12..15: offset for image 1 (32-bit)
- Bytes 16..: JPEG bytes for image 0 followed by JPEG bytes for image 1

## Collage generation

Collage images are built in groups of up to 25 input images. Each image is resized to `64x48` and pasted into a `5x5` grid, producing a final collage size of `320x240`.

- If there are fewer than 25 images in a collage group, remaining cells are filled with black.
- If more than 25 input images are provided, additional collage images are created.
- The first two bytes of `output.bin` report the collage image count and the total image count.

## Notes

- The script validates JPEG quality and file existence.
- The `output.bin` file always uses the requested JPEG quality for each image's stored JPEG bytes.
- Image caching: if both `output/resized/` and `output/quality/` versions of an image exist, they are reused instead of being regenerated (unless `-c` is used).
