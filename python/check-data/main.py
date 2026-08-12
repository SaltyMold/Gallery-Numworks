#!/usr/bin/env python3
"""Inspect output.nwip and print header information and image offsets (with sizes in KB).

Usage:
    python inspect_output_bin.py [path/to/output.nwip]

If no path is provided the script will try to read `output.nwip` from the
current working directory.
"""
import argparse
import os
import struct
import sys


def read_uint32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def main() -> int:
    parser = argparse.ArgumentParser(description="Print preview count, total images and offset/size pairs from output.nwip")
    parser.add_argument("file", nargs="?", default="output.nwip", help="Path to output.nwip (default: ./output.nwip)")
    args = parser.parse_args()

    path = args.file
    if not os.path.isfile(path):
        print(f"Error: file not found: {path}", file=sys.stderr)
        return 2

    with open(path, "rb") as f:
        data = f.read()

    if len(data) < 8:
        print("Error: file is too small to contain expected header.")
        return 3

    preview_count = read_uint32(data, 0)
    total_images = read_uint32(data, 4)

    print(f"Preview count: {preview_count}")
    print(f"Total images: {total_images}")

    expected_header_size = 8 + total_images * 8
    if len(data) < expected_header_size:
        print(f"Warning: file is smaller ({len(data)} bytes) than expected header size ({expected_header_size} bytes).")

    for i in range(total_images):
        # ensure we don't attempt to read past EOF
        off_pos = 8 + i * 8
        if off_pos + 8 > len(data):
            print(f"Offset {i}: (missing - header truncated)")
            continue
        offset = read_uint32(data, off_pos)
        size = read_uint32(data, off_pos + 4)
        size_kb = size / 1024.0
        print(f"Offset {i}: {offset} ({size} bytes, {size_kb:.1f} KB)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
