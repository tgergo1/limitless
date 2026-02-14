#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
VERSION_RE = re.compile(r"^([0-9]+)\.([0-9]+)\.([0-9]+)$")
PRERELEASE_RE = re.compile(r"^(rc|beta)([0-9A-Za-z.-]*)$")


def fail(msg: str) -> None:
    print(f"version check failed: {msg}", file=sys.stderr)
    raise SystemExit(1)


def read_text(path: pathlib.Path) -> str:
    return path.read_text(encoding="utf-8")


def resolve_version_file() -> pathlib.Path:
    primary = ROOT / "LIMITLESS_VERSION.txt"
    legacy = ROOT / "VERSION"
    if primary.is_file():
        return primary
    if legacy.is_file():
        return legacy
    fail("missing LIMITLESS_VERSION.txt (legacy fallback: VERSION)")
    return primary


def parse_version_file() -> str:
    version_path = resolve_version_file()
    v = read_text(version_path).strip()
    if not VERSION_RE.match(v):
        fail(f"{version_path.name} has invalid format: {v}")
    return v


def parse_header_version() -> str:
    text = read_text(ROOT / "limitless.h")
    m1 = re.search(r"^#define\s+LIMITLESS_VERSION_MAJOR\s+([0-9]+)\s*$", text, re.M)
    m2 = re.search(r"^#define\s+LIMITLESS_VERSION_MINOR\s+([0-9]+)\s*$", text, re.M)
    m3 = re.search(r"^#define\s+LIMITLESS_VERSION_PATCH\s+([0-9]+)\s*$", text, re.M)
    ms = re.search(r"^#define\s+LIMITLESS_VERSION_STRING\s+\"([0-9]+\.[0-9]+\.[0-9]+)\"\s*$", text, re.M)
    if not (m1 and m2 and m3 and ms):
        fail("limitless.h missing version macros")
    joined = f"{m1.group(1)}.{m2.group(1)}.{m3.group(1)}"
    if joined != ms.group(1):
        fail(f"limitless.h numeric/string mismatch: {joined} vs {ms.group(1)}")
    return joined


def parse_cmake_version() -> str:
    text = read_text(ROOT / "CMakeLists.txt")
    m = re.search(r"project\(limitless\s+VERSION\s+\"([0-9]+\.[0-9]+\.[0-9]+)\"", text)
    if m:
        return m.group(1)
    m_var = re.search(r"project\(limitless\s+VERSION\s+\"\$\{LIMITLESS_VERSION_RAW\}\"", text)
    if m_var:
        return parse_version_file()
    fail("CMakeLists.txt missing project(version)")
    return ""


def parse_vcpkg_version() -> str:
    data = json.loads(read_text(ROOT / "packaging" / "vcpkg" / "ports" / "limitless" / "vcpkg.json"))
    v = data.get("version-string", "")
    if not VERSION_RE.match(v):
        fail("vcpkg.json version-string invalid")
    return v


def main() -> None:
    expected = parse_version_file()
    checks = {
        "limitless.h": parse_header_version(),
        "CMakeLists.txt": parse_cmake_version(),
        "packaging/vcpkg/ports/limitless/vcpkg.json": parse_vcpkg_version(),
    }

    for name, got in checks.items():
        if got != expected:
            fail(f"{name} version {got} does not match version file {expected}")

    if len(sys.argv) > 1:
        tag = sys.argv[1].strip()
        if tag.startswith("refs/tags/"):
            tag = tag.split("/", 2)[2]
        if not tag.startswith("v"):
            fail(f"tag must start with 'v': {tag}")
        tag_version = tag[1:]
        if tag_version == expected:
            pass
        elif tag_version.startswith(expected + "-"):
            prerelease = tag_version[len(expected) + 1 :]
            if not PRERELEASE_RE.match(prerelease):
                fail(
                    "tag prerelease suffix must start with rc or beta, "
                    f"got: {tag_version}"
                )
        else:
            fail(f"tag version {tag_version} does not match version file {expected}")

    print(f"version check ok: {expected}")


if __name__ == "__main__":
    main()
