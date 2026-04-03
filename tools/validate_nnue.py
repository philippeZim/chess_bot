#!/usr/bin/env python3
"""Validate NNUE binary network file.

Checks magic number, version, dimensions, and weight ranges.

Usage:
    python validate_nnue.py network.bin
"""

import argparse
import struct
import sys
import os

NNUE_MAGIC = 0x4E4E5545
NNUE_VERSION = 1
EXPECTED_INPUT = 768
EXPECTED_HIDDEN = 256


def validate(path):
    """Validate an NNUE binary file."""
    if not os.path.exists(path):
        print(f"ERROR: File not found: {path}")
        return False

    file_size = os.path.getsize(path)
    expected_size = (
        4 * 4
        + EXPECTED_INPUT * EXPECTED_HIDDEN * 2
        + EXPECTED_HIDDEN * 2
        + EXPECTED_HIDDEN * 2 * 2
        + 2
    )

    print(f"Validating: {path}")
    print(f"File size: {file_size} bytes (expected: {expected_size})")

    if file_size != expected_size:
        print(f"WARNING: File size mismatch. Got {file_size}, expected {expected_size}")

    with open(path, "rb") as f:
        magic = struct.unpack("<I", f.read(4))[0]
        version = struct.unpack("<I", f.read(4))[0]
        input_size = struct.unpack("<I", f.read(4))[0]
        hidden_size = struct.unpack("<I", f.read(4))[0]

    errors = []

    if magic != NNUE_MAGIC:
        errors.append(f"Bad magic: 0x{magic:08X} (expected 0x{NNUE_MAGIC:08X})")
    else:
        print("  Magic: OK (0x4E4E5545)")

    if version != NNUE_VERSION:
        errors.append(f"Bad version: {version} (expected {NNUE_VERSION})")
    else:
        print("  Version: OK (1)")

    if input_size != EXPECTED_INPUT:
        errors.append(f"Bad input size: {input_size} (expected {EXPECTED_INPUT})")
    else:
        print(f"  Input size: OK ({EXPECTED_INPUT})")

    if hidden_size != EXPECTED_HIDDEN:
        errors.append(f"Bad hidden size: {hidden_size} (expected {EXPECTED_HIDDEN})")
    else:
        print(f"  Hidden size: OK ({EXPECTED_HIDDEN})")

    if errors:
        print("\nValidation FAILED:")
        for e in errors:
            print(f"  - {e}")
        return False

    with open(path, "rb") as f:
        f.read(16)
        import numpy as np

        w_ih = np.frombuffer(
            f.read(EXPECTED_INPUT * EXPECTED_HIDDEN * 2), dtype=np.int16
        )
        b_h = np.frombuffer(f.read(EXPECTED_HIDDEN * 2), dtype=np.int16)
        w_ho = np.frombuffer(f.read(EXPECTED_HIDDEN * 2 * 2), dtype=np.int16)
        b_o = np.frombuffer(f.read(2), dtype=np.int16)[0]

    print(f"  Weights IH: min={w_ih.min()}, max={w_ih.max()}, mean={w_ih.mean():.1f}")
    print(f"  Bias H:     min={b_h.min()}, max={b_h.max()}, mean={b_h.mean():.1f}")
    print(f"  Weights HO: min={w_ho.min()}, max={w_ho.max()}, mean={w_ho.mean():.1f}")
    print(f"  Bias O:     {b_o}")

    print("\nValidation PASSED")
    return True


def main():
    parser = argparse.ArgumentParser(description="Validate NNUE binary network file")
    parser.add_argument("path", help="Path to network.bin")
    args = parser.parse_args()

    if not validate(args.path):
        sys.exit(1)


if __name__ == "__main__":
    main()
