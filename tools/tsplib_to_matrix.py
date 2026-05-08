#!/usr/bin/env python3
"""将 TSPLIB 对称 TSP 实例转换为本项目使用的距离矩阵格式。"""

from __future__ import annotations

import argparse
import math
import sys
from pathlib import Path


SECTION_NAMES = {
    "NODE_COORD_SECTION",
    "EDGE_WEIGHT_SECTION",
    "DISPLAY_DATA_SECTION",
    "TOUR_SECTION",
    "EOF",
}


def normalize_key(key: str) -> str:
    """TSPLIB 字段名大小写不敏感，这里统一成大写下划线形式。"""
    return key.strip().upper().replace("-", "_")


def parse_header_line(line: str) -> tuple[str, str] | None:
    """解析 TSPLIB 头部字段，兼容 `KEY: VALUE` 和 `KEY VALUE` 两种写法。"""
    stripped = line.strip()
    if not stripped:
        return None

    if ":" in stripped:
        key, value = stripped.split(":", 1)
        return normalize_key(key), value.strip()

    parts = stripped.split(maxsplit=1)
    if len(parts) == 2:
        return normalize_key(parts[0]), parts[1].strip()
    return normalize_key(parts[0]), ""


def read_tsplib(path: Path) -> tuple[dict[str, str], dict[str, list[list[str]]]]:
    """读取 TSPLIB 文件，返回头部字段和各 section 的原始 token。"""
    header: dict[str, str] = {}
    sections: dict[str, list[list[str]]] = {}
    current_section: str | None = None

    for raw_line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw_line.strip()
        if not line:
            continue

        upper_line = line.upper()
        if upper_line == "EOF":
            break
        if upper_line in SECTION_NAMES:
            current_section = upper_line
            sections.setdefault(current_section, [])
            continue

        if current_section is None:
            parsed = parse_header_line(line)
            if parsed is not None:
                key, value = parsed
                header[key] = value
        else:
            sections.setdefault(current_section, []).append(line.split())

    return header, sections


def require_dimension(header: dict[str, str]) -> int:
    """读取 DIMENSION 字段，并做基本合法性检查。"""
    if "DIMENSION" not in header:
        raise ValueError("missing DIMENSION")
    dimension = int(header["DIMENSION"])
    if dimension < 3:
        raise ValueError("DIMENSION must be at least 3")
    return dimension


def read_node_coordinates(
    header: dict[str, str],
    sections: dict[str, list[list[str]]],
) -> list[list[float]]:
    """读取 NODE_COORD_SECTION，并按节点编号排序为 0-based 坐标数组。"""
    n = require_dimension(header)
    rows = sections.get("NODE_COORD_SECTION")
    if rows is None:
        raise ValueError("missing NODE_COORD_SECTION")

    coords: dict[int, tuple[float, float]] = {}
    for row in rows:
        if len(row) < 3:
            raise ValueError(f"invalid NODE_COORD_SECTION row: {' '.join(row)}")
        node_id = int(row[0])
        if node_id < 1 or node_id > n:
            raise ValueError(f"node id out of range: {node_id}")
        coords[node_id - 1] = [float(value) for value in row[1:]]

    if len(coords) != n:
        raise ValueError(f"expected {n} coordinates, got {len(coords)}")
    return [coords[i] for i in range(n)]


def nearest_integer(value: float) -> int:
    """TSPLIB 的 nint：四舍五入到最近整数。"""
    return int(value + 0.5)


def geo_to_radians(value: float) -> float:
    """TSPLIB GEO 坐标转换：ddd.mm 格式转换到弧度。"""
    degrees = int(value)
    minutes = value - degrees
    return math.pi * (degrees + 5.0 * minutes / 3.0) / 180.0


def require_coordinate_size(
    edge_weight_type: str,
    first: list[float],
    second: list[float],
    size: int,
) -> None:
    """确保坐标维度满足距离类型要求，避免静默忽略输入数据。"""
    if len(first) < size or len(second) < size:
        raise ValueError(f"{edge_weight_type} requires at least {size} coordinate values")


def coordinate_distance(
    edge_weight_type: str,
    first: list[float],
    second: list[float],
) -> int:
    """根据 EDGE_WEIGHT_TYPE 计算两个坐标点之间的 TSPLIB 整数距离。"""
    if edge_weight_type in {"EUC_3D", "CEIL_3D", "MAN_3D", "MAX_3D"}:
        require_coordinate_size(edge_weight_type, first, second, 3)
        diffs = [first[idx] - second[idx] for idx in range(3)]
        if edge_weight_type == "EUC_3D":
            return nearest_integer(math.sqrt(sum(value * value for value in diffs)))
        if edge_weight_type == "CEIL_3D":
            return int(math.ceil(math.sqrt(sum(value * value for value in diffs))))
        if edge_weight_type == "MAN_3D":
            return nearest_integer(sum(abs(value) for value in diffs))
        if edge_weight_type == "MAX_3D":
            return nearest_integer(max(abs(value) for value in diffs))

    require_coordinate_size(edge_weight_type, first, second, 2)
    x1, y1 = first[0], first[1]
    x2, y2 = second[0], second[1]
    dx = x1 - x2
    dy = y1 - y2

    if edge_weight_type == "EUC_2D":
        return nearest_integer(math.sqrt(dx * dx + dy * dy))
    if edge_weight_type == "CEIL_2D":
        return int(math.ceil(math.sqrt(dx * dx + dy * dy)))
    if edge_weight_type == "FLOOR_2D":
        return int(math.floor(math.sqrt(dx * dx + dy * dy)))
    if edge_weight_type == "MAN_2D":
        return nearest_integer(abs(dx) + abs(dy))
    if edge_weight_type == "MAX_2D":
        return nearest_integer(max(abs(dx), abs(dy)))
    if edge_weight_type == "ATT":
        rij = math.sqrt((dx * dx + dy * dy) / 10.0)
        tij = int(rij + 0.5)
        return tij if tij >= rij else tij + 1
    if edge_weight_type == "GEO":
        radius = 6378.388
        lat1 = geo_to_radians(x1)
        lon1 = geo_to_radians(y1)
        lat2 = geo_to_radians(x2)
        lon2 = geo_to_radians(y2)
        q1 = math.cos(lon1 - lon2)
        q2 = math.cos(lat1 - lat2)
        q3 = math.cos(lat1 + lat2)
        cosine = 0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)
        cosine = min(1.0, max(-1.0, cosine))
        return int(radius * math.acos(cosine) + 1.0)

    raise ValueError(f"unsupported coordinate EDGE_WEIGHT_TYPE: {edge_weight_type}")


def build_coordinate_matrix(
    header: dict[str, str],
    sections: dict[str, list[list[str]]],
) -> list[list[float]]:
    """将坐标型 TSPLIB 实例转换成完整对称距离矩阵。"""
    edge_weight_type = normalize_key(header.get("EDGE_WEIGHT_TYPE", "EUC_2D"))
    coords = read_node_coordinates(header, sections)
    n = len(coords)
    matrix: list[list[float]] = [[0.0 for _ in range(n)] for _ in range(n)]

    for i in range(n):
        for j in range(i + 1, n):
            distance = float(coordinate_distance(edge_weight_type, coords[i], coords[j]))
            matrix[i][j] = distance
            matrix[j][i] = distance
    return matrix


def edge_weight_tokens(sections: dict[str, list[list[str]]]) -> list[float]:
    """把 EDGE_WEIGHT_SECTION 中可能跨行的权重压平成一个数字列表。"""
    rows = sections.get("EDGE_WEIGHT_SECTION")
    if rows is None:
        raise ValueError("missing EDGE_WEIGHT_SECTION")

    tokens: list[float] = []
    for row in rows:
        for token in row:
            tokens.append(float(token))
    return tokens


def fill_upper_row(matrix: list[list[float]], values: list[float], include_diagonal: bool) -> None:
    """填充 UPPER_ROW / UPPER_DIAG_ROW 格式。"""
    n = len(matrix)
    index = 0
    for i in range(n):
        start = i if include_diagonal else i + 1
        for j in range(start, n):
            if index >= len(values):
                raise ValueError("EDGE_WEIGHT_SECTION has too few values")
            matrix[i][j] = values[index]
            matrix[j][i] = values[index]
            index += 1
    if index != len(values):
        raise ValueError("EDGE_WEIGHT_SECTION has too many values")


def fill_lower_row(matrix: list[list[float]], values: list[float], include_diagonal: bool) -> None:
    """填充 LOWER_ROW / LOWER_DIAG_ROW 格式。"""
    n = len(matrix)
    index = 0
    for i in range(n):
        end = i + 1 if include_diagonal else i
        for j in range(end):
            if index >= len(values):
                raise ValueError("EDGE_WEIGHT_SECTION has too few values")
            matrix[i][j] = values[index]
            matrix[j][i] = values[index]
            index += 1
    if index != len(values):
        raise ValueError("EDGE_WEIGHT_SECTION has too many values")


def fill_upper_col(matrix: list[list[float]], values: list[float], include_diagonal: bool) -> None:
    """填充 UPPER_COL / UPPER_DIAG_COL 格式。"""
    n = len(matrix)
    index = 0
    for j in range(n):
        end = j + 1 if include_diagonal else j
        for i in range(end):
            if index >= len(values):
                raise ValueError("EDGE_WEIGHT_SECTION has too few values")
            matrix[i][j] = values[index]
            matrix[j][i] = values[index]
            index += 1
    if index != len(values):
        raise ValueError("EDGE_WEIGHT_SECTION has too many values")


def fill_lower_col(matrix: list[list[float]], values: list[float], include_diagonal: bool) -> None:
    """填充 LOWER_COL / LOWER_DIAG_COL 格式。"""
    n = len(matrix)
    index = 0
    for j in range(n):
        start = j if include_diagonal else j + 1
        for i in range(start, n):
            if index >= len(values):
                raise ValueError("EDGE_WEIGHT_SECTION has too few values")
            matrix[i][j] = values[index]
            matrix[j][i] = values[index]
            index += 1
    if index != len(values):
        raise ValueError("EDGE_WEIGHT_SECTION has too many values")


def build_explicit_matrix(
    header: dict[str, str],
    sections: dict[str, list[list[str]]],
) -> list[list[float]]:
    """解析 EDGE_WEIGHT_TYPE=EXPLICIT 的常见矩阵存储格式。"""
    n = require_dimension(header)
    values = edge_weight_tokens(sections)
    fmt = normalize_key(header.get("EDGE_WEIGHT_FORMAT", "FULL_MATRIX"))
    matrix: list[list[float]] = [[0.0 for _ in range(n)] for _ in range(n)]

    if fmt == "FULL_MATRIX":
        expected = n * n
        if len(values) != expected:
            raise ValueError(f"FULL_MATRIX expected {expected} values, got {len(values)}")
        index = 0
        for i in range(n):
            for j in range(n):
                matrix[i][j] = values[index]
                index += 1
        return matrix

    if fmt == "UPPER_ROW":
        fill_upper_row(matrix, values, include_diagonal=False)
        return matrix
    if fmt == "UPPER_DIAG_ROW":
        fill_upper_row(matrix, values, include_diagonal=True)
        return matrix
    if fmt == "LOWER_ROW":
        fill_lower_row(matrix, values, include_diagonal=False)
        return matrix
    if fmt == "LOWER_DIAG_ROW":
        fill_lower_row(matrix, values, include_diagonal=True)
        return matrix
    if fmt == "UPPER_COL":
        fill_upper_col(matrix, values, include_diagonal=False)
        return matrix
    if fmt == "UPPER_DIAG_COL":
        fill_upper_col(matrix, values, include_diagonal=True)
        return matrix
    if fmt == "LOWER_COL":
        fill_lower_col(matrix, values, include_diagonal=False)
        return matrix
    if fmt == "LOWER_DIAG_COL":
        fill_lower_col(matrix, values, include_diagonal=True)
        return matrix

    raise ValueError(f"unsupported EDGE_WEIGHT_FORMAT: {fmt}")


def build_matrix(header: dict[str, str], sections: dict[str, list[list[str]]]) -> list[list[float]]:
    """根据 TSPLIB 头部字段选择坐标型或显式矩阵型转换流程。"""
    problem_type = normalize_key(header.get("TYPE", "TSP"))
    if problem_type != "TSP":
        raise ValueError(f"only symmetric TYPE=TSP is supported, got {problem_type}")

    edge_weight_type = normalize_key(header.get("EDGE_WEIGHT_TYPE", "EUC_2D"))
    if edge_weight_type == "EXPLICIT":
        return build_explicit_matrix(header, sections)
    return build_coordinate_matrix(header, sections)


def check_symmetric(matrix: list[list[float]], tolerance: float) -> None:
    """当前 C++ 求解器只支持对称 TSP，因此转换后默认检查矩阵对称性。"""
    n = len(matrix)
    for i in range(n):
        for j in range(i + 1, n):
            if abs(matrix[i][j] - matrix[j][i]) > tolerance:
                raise ValueError(f"matrix is not symmetric at ({i + 1}, {j + 1})")


def format_weight(value: float) -> str:
    """以求解器可读的形式输出权重；整数权重不带小数。"""
    if math.isinf(value):
        return "x"
    if abs(value - round(value)) <= 1e-9:
        return str(int(round(value)))
    return f"{value:.10g}"


def write_matrix(path: Path | None, matrix: list[list[float]]) -> None:
    """写出本项目矩阵格式；path 为 None 时输出到 stdout。"""
    lines = [str(len(matrix))]
    for row in matrix:
        lines.append(" ".join(format_weight(value) for value in row))
    text = "\n".join(lines) + "\n"

    if path is None:
        sys.stdout.write(text)
        return

    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def output_path_for(input_path: Path, output_dir: Path) -> Path:
    """多文件转换时，以输入文件名为基础生成 .txt 矩阵文件。"""
    return output_dir / f"{input_path.stem}.txt"


def display_path(path: Path) -> str:
    """批处理清单尽量写相对路径，便于项目内迁移。"""
    try:
        return str(path.relative_to(Path.cwd()))
    except ValueError:
        return str(path)


def convert_one(input_path: Path, output_path: Path | None, tolerance: float) -> Path | None:
    """转换单个 TSPLIB 文件，返回实际写出的路径；stdout 模式返回 None。"""
    header, sections = read_tsplib(input_path)
    matrix = build_matrix(header, sections)
    check_symmetric(matrix, tolerance)
    write_matrix(output_path, matrix)
    return output_path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("inputs", nargs="+", help="TSPLIB .tsp files")
    parser.add_argument("--output", help="single output matrix file; only valid for one input")
    parser.add_argument("--output-dir", help="directory for converted matrix files")
    parser.add_argument("--batch-list", help="write converted output paths to this list file")
    parser.add_argument("--symmetry-tolerance", type=float, default=1e-9)
    args = parser.parse_args()

    inputs = [Path(value) for value in args.inputs]
    if args.output and len(inputs) != 1:
        parser.error("--output can only be used with one input")
    if len(inputs) > 1 and not args.output_dir:
        parser.error("multiple inputs require --output-dir")
    if args.batch_list and not (args.output or args.output_dir):
        parser.error("--batch-list requires --output or --output-dir")

    output_dir = Path(args.output_dir) if args.output_dir else None
    written_paths: list[Path] = []

    for input_path in inputs:
        # 单文件没有指定输出时走 stdout，方便管道调试；批量转换必须写入目录。
        if args.output:
            output_path: Path | None = Path(args.output)
        elif output_dir is not None:
            output_path = output_path_for(input_path, output_dir)
        else:
            output_path = None

        written = convert_one(input_path, output_path, args.symmetry_tolerance)
        if written is not None:
            written_paths.append(written)
            print(f"{display_path(input_path)} -> {display_path(written)}", file=sys.stderr)

    if args.batch_list:
        list_path = Path(args.batch_list)
        list_path.parent.mkdir(parents=True, exist_ok=True)
        with list_path.open("w", encoding="utf-8") as output:
            output.write("# TSPLIB 转换后的实例清单\n")
            for path in written_paths:
                output.write(display_path(path) + "\n")
        print(f"batch list: {display_path(list_path)}", file=sys.stderr)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
