#!/usr/bin/env python3
"""NNUE training script for chess bot.

Generates synthetic training data and trains a Half-KRNN network
(768 -> 256 -> 1) using PyTorch.

Usage:
    python train_nnue.py [--epochs N] [--batch-size N] [--lr LR] [--output PATH]
"""

import argparse
import struct
import sys
import os
import random
import math

try:
    import torch
    import torch.nn as nn
    import torch.optim as optim

    HAS_TORCH = True
except ImportError:
    HAS_TORCH = False

NNUE_INPUT = 768
NNUE_HIDDEN = 256
NNUE_OUTPUT = 1
NNUE_MAGIC = 0x4E4E5545
NNUE_VERSION = 1


def generate_synthetic_data(num_samples=10000):
    """Generate synthetic training data with known patterns."""
    random.seed(42)
    X = []
    y = []

    piece_values = {0: 100, 1: 320, 2: 330, 3: 500, 4: 900, 5: 0}

    for _ in range(num_samples):
        features = [0.0] * NNUE_INPUT
        material_score = 0

        for sq in range(64):
            if random.random() < 0.35:
                color = random.choice([0, 1])
                pt = random.choices(range(6), weights=[40, 8, 8, 4, 2, 2])[0]

                sign = 1 if color == 0 else -1
                feat_idx = color * 384 + pt * 64 + sq
                features[feat_idx] = 1.0
                material_score += sign * piece_values[pt]

        noise = random.gauss(0, 15)
        target = material_score + noise

        X.append(features)
        y.append([target])

    return X, y


def generate_feature_vector(board_fen=None):
    """Convert a board position to a 768-dimensional feature vector.

    For use with real game data. Returns one-hot encoded features.
    """
    features = [0.0] * NNUE_INPUT

    if board_fen is None:
        return features

    piece_map = {
        "P": (0, 0),
        "N": (0, 1),
        "B": (0, 2),
        "R": (0, 3),
        "Q": (0, 4),
        "K": (0, 5),
        "p": (1, 0),
        "n": (1, 1),
        "b": (1, 2),
        "r": (1, 3),
        "q": (1, 4),
        "k": (1, 5),
    }

    try:
        placement = board_fen.split()[0]
        sq = 0
        for c in placement:
            if c.isdigit():
                sq += int(c)
            elif c == "/":
                continue
            elif c in piece_map:
                color, pt = piece_map[c]
                feat_idx = color * 384 + pt * 64 + sq
                features[feat_idx] = 1.0
                sq += 1
            else:
                sq += 1
    except (IndexError, ValueError):
        pass

    return features


class NNUE(nn.Module):
    """Half-KRNN architecture: 768 -> 256 (ReLU) -> 1."""

    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(NNUE_INPUT, NNUE_HIDDEN)
        self.fc2 = nn.Linear(NNUE_HIDDEN, NNUE_OUTPUT)

    def forward(self, x):
        x = torch.relu(self.fc1(x))
        x = self.fc2(x)
        return x.squeeze(-1)


def export_weights(model, path):
    """Export trained weights to binary format for C engine."""
    with torch.no_grad():
        w_ih = model.fc1.weight.numpy().T
        b_h = model.fc1.bias.numpy()
        w_ho = model.fc2.weight.numpy().T
        b_o = model.fc2.bias.numpy()[0]

    w_ih_q = np.clip(np.round(w_ih * 256).astype(np.int16), -127, 127)
    b_h_q = np.clip(np.round(b_h * 256).astype(np.int16), -127, 127)
    w_ho_q = np.clip(np.round(w_ho * 256).astype(np.int16), -127, 127)
    b_o_q = np.clip(np.round(b_o * 256).astype(np.int16), -127, 127)

    with open(path, "wb") as f:
        f.write(struct.pack("<I", NNUE_MAGIC))
        f.write(struct.pack("<I", NNUE_VERSION))
        f.write(struct.pack("<I", NNUE_INPUT))
        f.write(struct.pack("<I", NNUE_HIDDEN))
        f.write(w_ih_q.tobytes())
        f.write(b_h_q.tobytes())
        f.write(w_ho_q.tobytes())
        f.write(b_o_q.tobytes())


def train(args):
    """Train the NNUE network."""
    if not HAS_TORCH:
        print("Error: PyTorch is required. Install with: pip install torch")
        sys.exit(1)

    global np
    import numpy as np

    print(f"Generating {args.samples} synthetic training samples...")
    X, y = generate_synthetic_data(args.samples)

    X_tensor = torch.tensor(X, dtype=torch.float32)
    y_tensor = torch.tensor(y, dtype=torch.float32).squeeze(-1)

    dataset = torch.utils.data.TensorDataset(X_tensor, y_tensor)
    loader = torch.utils.data.DataLoader(
        dataset, batch_size=args.batch_size, shuffle=True
    )

    model = NNUE()
    criterion = nn.MSELoss()
    optimizer = optim.AdamW(model.parameters(), lr=args.lr, weight_decay=0.01)
    scheduler = optim.lr_scheduler.StepLR(optimizer, step_size=10, gamma=0.5)

    print(f"\nTraining for {args.epochs} epochs...")
    print(f"Architecture: {NNUE_INPUT} -> {NNUE_HIDDEN} (ReLU) -> {NNUE_OUTPUT}")
    print(f"Batch size: {args.batch_size}, LR: {args.lr}")
    print("-" * 60)

    for epoch in range(args.epochs):
        model.train()
        total_loss = 0.0
        n_batches = 0

        for batch_X, batch_y in loader:
            optimizer.zero_grad()
            output = model(batch_X)
            loss = criterion(output, batch_y)
            loss.backward()
            optimizer.step()
            total_loss += loss.item()
            n_batches += 1

        scheduler.step()
        avg_loss = total_loss / n_batches
        print(
            f"Epoch {epoch + 1:3d}/{args.epochs} | Loss: {avg_loss:10.2f} | LR: {scheduler.get_last_lr()[0]:.6f}"
        )

    model.eval()
    with torch.no_grad():
        test_output = model(X_tensor[:100])
        test_loss = criterion(test_output, y_tensor[:100]).item()
    print("-" * 60)
    print(f"Final test loss: {test_loss:.2f}")

    os.makedirs(
        os.path.dirname(args.output) if os.path.dirname(args.output) else ".",
        exist_ok=True,
    )
    export_weights(model, args.output)
    print(f"Weights exported to {args.output}")

    return model


def main():
    parser = argparse.ArgumentParser(description="Train NNUE network for chess bot")
    parser.add_argument(
        "--epochs", type=int, default=30, help="Number of training epochs"
    )
    parser.add_argument(
        "--batch-size", type=int, default=1024, help="Training batch size"
    )
    parser.add_argument("--lr", type=float, default=0.001, help="Learning rate")
    parser.add_argument(
        "--samples", type=int, default=10000, help="Number of synthetic samples"
    )
    parser.add_argument(
        "--output", type=str, default="data/nnue/network.bin", help="Output path"
    )
    args = parser.parse_args()

    train(args)


if __name__ == "__main__":
    main()
