#!/usr/bin/env python3
"""Download classic TSP benchmark datasets and create batch lists."""

from __future__ import annotations

import argparse
import gzip
import re
import shutil
import ssl
import tarfile
import urllib.request
from pathlib import Path
from urllib.parse import urljoin, urlparse


TSPLIB_ARCHIVE = "https://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/tsp/ALL_tsp.tar.gz"
VLSI_ARCHIVE = "https://www.math.uwaterloo.ca/tsp/vlsi/vlsi_tsp.tgz"
NATIONAL_BASE = "https://www.math.uwaterloo.ca/tsp/world/"
NATIONAL_INSTANCES = [
    "ar9152",
    "bm33708",
    "ch71009",
    "dj38",
    "eg7146",
    "fi10639",
    "gr9882",
    "ho14473",
    "ei8246",
    "ja9847",
    "kz9976",
    "lu980",
    "mo14185",
    "nu3496",
    "mu1979",
    "pm8079",
    "qa194",
    "rw1621",
    "sw24978",
    "tz6117",
    "uy734",
    "vm22775",
    "wi29",
    "ym7663",
    "zi929",
    "ca4663",
    "it16862",
]
DIMACS_DOWNLOAD_PAGE = "https://archive.dimacs.rutgers.edu/Challenges/TSP/download.html"
SSL_CONTEXT: ssl.SSLContext | None = None


def request(url: str) -> urllib.request.Request:
    return urllib.request.Request(url, headers={"User-Agent": "TSP benchmark downloader"})


def download(url: str, path: Path, force: bool) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if path.exists() and not force:
        print(f"[skip] {path}")
        return
    print(f"[get]  {url}")
    with urllib.request.urlopen(request(url), timeout=120, context=SSL_CONTEXT) as response:
        with path.open("wb") as output:
            shutil.copyfileobj(response, output)


def safe_extract_tar(archive: Path, output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    root = output_dir.resolve()
    with tarfile.open(archive, "r:*") as tar:
        for member in tar.getmembers():
            target = (output_dir / member.name).resolve()
            if root != target and root not in target.parents:
                raise RuntimeError(f"unsafe tar member path: {member.name}")
        tar.extractall(output_dir)


def decompress_gzip_file(path: Path, force: bool) -> Path:
    if path.suffix != ".gz":
        return path
    output = path.with_suffix("")
    if output.exists() and not force:
        return output
    with gzip.open(path, "rb") as source:
        with output.open("wb") as target:
            shutil.copyfileobj(source, target)
    return output


def decompress_gzip_tree(root: Path, force: bool) -> None:
    for path in sorted(root.rglob("*.gz")):
        if tarfile.is_tarfile(path):
            continue
        decompress_gzip_file(path, force)


def archive_name(url: str) -> str:
    name = Path(urlparse(url).path).name
    if not name:
        raise ValueError(f"URL has no file name: {url}")
    return name


def download_archive(url: str, output_dir: Path, force: bool) -> None:
    raw_dir = output_dir / "_archives"
    archive_path = raw_dir / archive_name(url)
    download(url, archive_path, force)
    safe_extract_tar(archive_path, output_dir)
    decompress_gzip_tree(output_dir, force)


def download_national(output_dir: Path, force: bool) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    for name in NATIONAL_INSTANCES:
        url = urljoin(NATIONAL_BASE, f"{name}.tsp")
        download(url, output_dir / f"{name}.tsp", force)


def discover_dimacs_links() -> list[str]:
    with urllib.request.urlopen(request(DIMACS_DOWNLOAD_PAGE), timeout=120, context=SSL_CONTEXT) as response:
        html = response.read().decode("latin1")
    urls: list[str] = []
    for href in re.findall(r'href=["\']([^"\']+)["\']', html, flags=re.IGNORECASE):
        lowered = href.lower()
        if not lowered.endswith((".tar", ".tar.gz", ".tgz", ".zip", ".txt")):
            continue
        urls.append(urljoin(DIMACS_DOWNLOAD_PAGE, href))
    return sorted(dict.fromkeys(urls))


def download_dimacs(output_dir: Path, force: bool) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    page_path = output_dir / "download.html"
    download(DIMACS_DOWNLOAD_PAGE, page_path, force)
    links = discover_dimacs_links()
    if not links:
        print("[warn] no DIMACS archive links discovered; saved download.html for inspection")
        return
    raw_dir = output_dir / "_archives"
    for url in links:
        path = raw_dir / archive_name(url)
        download(url, path, force)
        if tarfile.is_tarfile(path):
            safe_extract_tar(path, output_dir)
        elif path.suffix == ".gz":
            decompress_gzip_file(path, force)
    decompress_gzip_tree(output_dir, force)


def write_batch(root: Path, dataset_dir: Path, name: str) -> None:
    paths = sorted(path for path in dataset_dir.rglob("*") if is_instance_file(path))
    batch_path = root / f"{name}.txt"
    with batch_path.open("w", encoding="utf-8") as output:
        output.write(f"# {name} benchmark instances\n")
        for path in paths:
            try:
                output.write(str(path.relative_to(Path.cwd())) + "\n")
            except ValueError:
                output.write(str(path) + "\n")
    print(f"[list] {batch_path} ({len(paths)} instances)")


def is_instance_file(path: Path) -> bool:
    if not path.is_file() or "_archives" in path.parts:
        return False
    if path.suffix == ".tsp":
        return True
    return re.match(r"^[ECM][0-9]+[kM]?\.[0-9]+$", path.name) is not None


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", default="data/classic", help="output directory")
    parser.add_argument(
        "--datasets",
        default="tsplib,national,vlsi,dimacs",
        help="comma-separated names: tsplib,national,vlsi,dimacs",
    )
    parser.add_argument("--force", action="store_true", help="redownload existing files")
    parser.add_argument("--insecure", action="store_true", help="disable TLS certificate verification")
    args = parser.parse_args()

    global SSL_CONTEXT
    if args.insecure:
        SSL_CONTEXT = ssl._create_unverified_context()

    root = Path(args.output)
    root.mkdir(parents=True, exist_ok=True)
    selected = {item.strip().lower() for item in args.datasets.split(",") if item.strip()}

    if "tsplib" in selected:
        download_archive(TSPLIB_ARCHIVE, root / "tsplib", args.force)
        write_batch(root, root / "tsplib", "batch-tsplib")
    if "national" in selected:
        download_national(root / "national", args.force)
        write_batch(root, root / "national", "batch-national")
    if "vlsi" in selected:
        download_archive(VLSI_ARCHIVE, root / "vlsi", args.force)
        write_batch(root, root / "vlsi", "batch-vlsi")
    if "dimacs" in selected:
        download_dimacs(root / "dimacs", args.force)
        write_batch(root, root / "dimacs", "batch-dimacs")

    all_paths = sorted(path for path in root.rglob("*") if is_instance_file(path))
    with (root / "batch-all.txt").open("w", encoding="utf-8") as output:
        output.write("# All downloaded classic TSP benchmark instances\n")
        for path in all_paths:
            try:
                output.write(str(path.relative_to(Path.cwd())) + "\n")
            except ValueError:
                output.write(str(path) + "\n")
    print(f"[list] {root / 'batch-all.txt'} ({len(all_paths)} instances)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
