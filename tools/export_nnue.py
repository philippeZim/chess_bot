#!/usr/bin/env python3
"""Export NNUE network from PyTorch to binary format for C engine.

Usage:
    python export_nnue.py --model model.pt --output network.bin
"""

import argparse
import struct
import os
import sys

try:
    import torch
    import torch.nn as nn

    HAS_TORCH = True
except ImportError:
    HAS_TORCH = False

NNUE_MAGIC = 0x4E4E5545
NNUE_VERSION = 1
NNUE_INPUT = 768
NNUE_HIDDEN = 256


class NNUE(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(NNUE_INPUT, NNUE_HIDDEN)
        self.fc2 = nn.Linear(NNUE_HIDDEN, 1)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        return self.fc2(x).squeeze(-1)


def export_pytorch(model_path, output_path):
    """Export a PyTorch model to binary format."""
    if not HAS_TORCH:
        print("Error: PyTorch required. pip install torch")
        sys.exit(1)

    model = NNUE()
    model.load_state_dict(torch.load(model_path, map_location="cpu", weights_only=True))
    model.eval()

    with torch.no_grad():
        w_ih = model.fc1.weight.numpy().T
        b_h = model.fc1.bias.numpy()
        w_ho = model.fc2.weight.numpy().T
        b_o = model.fc2.bias.numpy()[0]

    import numpy as np

    w_ih_q = np.clip(np.round(w_ih * 256).astype(np.int16), -127, 127)
    b_h_q = np.clip(np.round(b_h * 256).astype(np.int16), -127, 127)
    w_ho_q = np.clip(np.round(w_ho * 256).astype(np.int16), -127, 127)
    b_o_q = np.clip(np.round(b_o * 256).astype(np.int16), -127, 127)

    os.makedirs(
        os.path.dirname(output_path) if os.path.dirname(output_path) else ".",
        exist_ok=True,
    )

    with open(output_path, "wb") as f:
        f.write(struct.pack("<I", NNUE_MAGIC))
        f.write(struct.pack("<I", NNUE_VERSION))
        f.write(struct.pack("<I", NNUE_INPUT))
        f.write(struct.pack("<I", NNUE_HIDDEN))
        f.write(w_ih_q.tobytes())
        f.write(b_h_q.tobytes())
        f.write(w_ho_q.tobytes())
        f.write(b_o_q.tobytes())

    file_size = os.path.getsize(output_path)
    print(f"Exported {model_path} -> {output_path}")
    print(f"File size: {file_size} bytes")
    print(f"  Magic: 0x{NNUE_MAGIC:08X}")
    print(f"  Version: {NNUE_VERSION}")
    print(f"  Input: {NNUE_INPUT}, Hidden: {NNUE_HIDDEN}")


def main():
    parser = argparse.ArgumentParser(description="Export NNUE network to binary format")
    parser.add_argument("--model", required=True, help="PyTorch model path (.pt)")
    parser.add_argument("--output", required=True, help="Output binary path")
    args = parser.parse_args()
    export_pytorch(args.model, args.output)


if __name__ == "__main__":
    main()
