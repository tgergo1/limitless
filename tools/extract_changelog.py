#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
CHANGELOG = ROOT / "CHANGELOG.md"


def normalize(version: str) -> str:
    v = version.strip()
    if v.startswith("refs/tags/"):
        v = v.split("/", 2)[2]
    if v.startswith("v"):
        v = v[1:]
    return v


def extract(version: str) -> str:
    text = CHANGELOG.read_text(encoding="utf-8")
    lines = text.splitlines()
    header = re.compile(r"^##\s+\[([^\]]+)\]")
    start = -1
    end = len(lines)

    for i, line in enumerate(lines):
        m = header.match(line)
        if not m:
            continue
        if m.group(1) == version:
            start = i
            break

    if start < 0:
        return f"Release {version}\n\nSee CHANGELOG.md for details."

    for i in range(start + 1, len(lines)):
        if header.match(lines[i]):
            end = i
            break

    block = "\n".join(lines[start:end]).strip()
    return block if block else f"Release {version}"


def main() -> None:
    if len(sys.argv) != 2:
        raise SystemExit("usage: extract_changelog.py <tag-or-version>")
    print(extract(normalize(sys.argv[1])))


if __name__ == "__main__":
    main()
