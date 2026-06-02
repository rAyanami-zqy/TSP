#!/usr/bin/env python3
"""
Convert raw matrix format TSP instances to TSPLIB format.

Input format (raw matrix):
  - Lines starting with '#' are comments
  - First non-comment line: number of nodes (n)
  - Following n lines: distance matrix
  - Values can be numbers or 'x' (for blocked/absent edges)

Output format: TSPLIB EXPLICIT FULL_MATRIX
  - 'x' values are converted to a large sentinel (default 999999)
"""

import argparse
import os
import sys


def convert_file(input_path: str, output_path: str | None = None,
                 sentinel: int = 999999) -> str:
    """Convert a single raw matrix file to TSPLIB format.

    Returns the output file path.
    """
    with open(input_path) as f:
        lines = f.readlines()

    # Filter out comment lines
    data_lines = [l.strip() for l in lines if l.strip() and not l.strip().startswith('#')]

    if not data_lines:
        raise ValueError(f"No data found in {input_path}")

    try:
        n = int(data_lines[0])
    except ValueError:
        raise ValueError(f"First data line in {input_path} is not an integer: '{data_lines[0]}'")

    matrix_lines = data_lines[1:1 + n]

    if len(matrix_lines) != n:
        raise ValueError(
            f"Expected {n} matrix rows in {input_path}, got {len(matrix_lines)}")

    # Generate TSPLIB name from filename (without extension)
    basename = os.path.splitext(os.path.basename(input_path))[0]
    # Remove hyphens and other non-alphanumeric chars, replace with underscores
    name = basename.replace('-', '_')

    # Build TSPLIB content
    out_lines = [
        f"NAME: {name}",
        "TYPE: TSP",
        f"COMMENT: Converted from {input_path}",
        f"DIMENSION: {n}",
        "EDGE_WEIGHT_TYPE: EXPLICIT",
        "EDGE_WEIGHT_FORMAT: FULL_MATRIX",
        "EDGE_WEIGHT_SECTION",
    ]

    for row in matrix_lines:
        parts = row.split()
        if len(parts) != n:
            raise ValueError(
                f"Row has {len(parts)} entries, expected {n} in {input_path}: '{row}'")
        converted = [str(sentinel) if v.lower() == 'x' else v for v in parts]
        out_lines.append(" " + " ".join(converted))

    out_lines.append("EOF")

    if output_path is None:
        output_path = os.path.splitext(input_path)[0] + ".tsp"

    with open(output_path, 'w') as f:
        f.write("\n".join(out_lines) + "\n")

    return output_path


def convert_batch(batch_file: str, output_dir: str, sentinel: int = 999999):
    """Convert all files listed in a batch file."""
    with open(batch_file) as f:
        paths = [l.strip() for l in f if l.strip() and not l.strip().startswith('#')]

    os.makedirs(output_dir, exist_ok=True)
    converted = []

    for rel_path in paths:
        input_path = rel_path  # paths in batch are relative to project root or absolute
        if not os.path.isabs(input_path):
            # Try relative to batch file directory first
            batch_dir = os.path.dirname(batch_file)
            input_path = os.path.normpath(os.path.join(batch_dir, rel_path))

        if not os.path.exists(input_path):
            print(f"Warning: file not found, skipping: {input_path}", file=sys.stderr)
            continue

        basename = os.path.basename(input_path)
        out_name = os.path.splitext(basename)[0] + ".tsp"
        out_path = os.path.join(output_dir, out_name)

        try:
            convert_file(input_path, out_path, sentinel)
            converted.append(out_path)
            print(f"  {input_path} -> {out_path}")
        except Exception as e:
            print(f"  ERROR: {input_path}: {e}", file=sys.stderr)

    return converted


def convert_directory(input_dir: str, output_dir: str, sentinel: int = 999999):
    """Convert all .txt files in a directory."""
    os.makedirs(output_dir, exist_ok=True)
    converted = []

    for fname in sorted(os.listdir(input_dir)):
        if fname.endswith('.txt'):
            in_path = os.path.join(input_dir, fname)
            out_name = os.path.splitext(fname)[0] + ".tsp"
            out_path = os.path.join(output_dir, out_name)
            try:
                convert_file(in_path, out_path, sentinel)
                converted.append(out_path)
                print(f"  {in_path} -> {out_path}")
            except Exception as e:
                print(f"  ERROR: {in_path}: {e}", file=sys.stderr)

    return converted


def main():
    parser = argparse.ArgumentParser(
        description="Convert raw matrix TSP instances to TSPLIB format"
    )
    parser.add_argument(
        "input",
        help="Input file (raw matrix), batch file, or directory of .txt files"
    )
    parser.add_argument(
        "-o", "--output", default=None,
        help="Output file path (single file) or directory (batch/dir mode)"
    )
    parser.add_argument(
        "--batch", action="store_true",
        help="Treat input as a batch file listing multiple input files"
    )
    parser.add_argument(
        "--dir", action="store_true",
        help="Treat input as a directory, convert all .txt files"
    )
    parser.add_argument(
        "-s", "--sentinel", type=int, default=10000,
        help="Value to use for blocked edges ('x' entries), default 10000"
    )

    args = parser.parse_args()

    if args.batch:
        out_dir = args.output or os.path.join(os.path.dirname(args.input), "tsplib")
        converted = convert_batch(args.input, out_dir, args.sentinel)
        print(f"\nConverted {len(converted)} files to {out_dir}/")
    elif args.dir:
        out_dir = args.output or os.path.join(args.input, "tsplib")
        converted = convert_directory(args.input, out_dir, args.sentinel)
        print(f"\nConverted {len(converted)} files from {args.input}/ to {out_dir}/")
    else:
        out_path = args.output
        if out_path is None:
            base = os.path.splitext(args.input)[0]
            out_path = base + ".tsp"
        result = convert_file(args.input, out_path, args.sentinel)
        print(f"Converted: {args.input} -> {result}")


if __name__ == "__main__":
    main()
