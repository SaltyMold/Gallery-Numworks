#!/usr/bin/env python3

import sys
import os
import subprocess

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} image.jpg")
    sys.exit(1)

input_image = sys.argv[1]
output_dir = "output"

os.makedirs(output_dir, exist_ok=True)

for quality in range(100, 0, -5):
    output_file = os.path.join(output_dir, f"output_{quality}.jpg")

    subprocess.run([
        "magick",
        input_image,
        "-resize", "320x240!",
        "-quality", str(quality),
        output_file
    ], check=True)

    print(f"Created : {output_file}")

print("Finished.")