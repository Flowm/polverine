#!/usr/bin/env python3
"""
Build script to compress web assets for embedding in ESP32 firmware.
This script compresses HTML files using gzip for optimal flash storage.
"""

import os
import gzip
import sys
from pathlib import Path

def compress_file(input_path, output_path):
    """Compress a file using gzip compression."""
    try:
        with open(input_path, 'rb') as f_in:
            with gzip.open(output_path, 'wb', compresslevel=9) as f_out:
                f_out.write(f_in.read())

        # Get file sizes for reporting
        original_size = os.path.getsize(input_path)
        compressed_size = os.path.getsize(output_path)
        compression_ratio = (1 - compressed_size / original_size) * 100

        print(f"✓ {input_path} -> {output_path}")
        print(f"  Size: {original_size} bytes -> {compressed_size} bytes ({compression_ratio:.1f}% reduction)")
        return True

    except Exception as e:
        print(f"✗ Failed to compress {input_path}: {e}")
        return False

def main():
    """Main compression function."""
    script_dir = Path(__file__).parent
    web_dir = script_dir / "web"

    if not web_dir.exists():
        print(f"✗ Web directory not found: {web_dir}")
        sys.exit(1)

    print("Compressing web assets...")

    # Find all HTML files in the web directory
    html_files = list(web_dir.glob("*.html"))

    if not html_files:
        print(f"✗ No HTML files found in {web_dir}")
        sys.exit(1)

    success_count = 0
    total_files = len(html_files)

    for input_path in html_files:
        output_path = web_dir / f"{input_path.name}.gz"

        if compress_file(input_path, output_path):
            success_count += 1

    print(f"\nCompression complete: {success_count}/{total_files} files processed successfully.")

    if success_count == total_files:
        print("All web assets compressed successfully!")
        sys.exit(0)
    else:
        print("Some files failed to compress.")
        sys.exit(1)

if __name__ == "__main__":
    main()
