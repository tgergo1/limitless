#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import argparse
import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import sys
import urllib.request

ROOT = pathlib.Path(__file__).resolve().parents[1]
VERSION_RE = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+$")
REPO_OWNER = "tgergo1"
REPO_NAME = "limitless"
CHUNK_SIZE = 1024 * 1024


def fail(message: str) -> None:
    print(message, file=sys.stderr)
    raise SystemExit(1)


def read_text(path: pathlib.Path) -> str:
    return path.read_text(encoding="utf-8")


def write_text(path: pathlib.Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def write_json(path: pathlib.Path, data: object) -> None:
    write_text(path, json.dumps(data, indent=2) + "\n")


def normalize_version(version: str) -> str:
    normalized = version[1:] if version.startswith("v") else version
    if not VERSION_RE.match(normalized):
        fail(f"expected stable MAJOR.MINOR.PATCH version, got: {version}")
    return normalized


def source_url(version: str) -> str:
    return f"https://github.com/{REPO_OWNER}/{REPO_NAME}/archive/refs/tags/v{version}.tar.gz"


def hash_url(url: str) -> dict[str, str]:
    sha256 = hashlib.sha256()
    sha512 = hashlib.sha512()
    with urllib.request.urlopen(url) as response:
        while True:
            chunk = response.read(CHUNK_SIZE)
            if not chunk:
                break
            sha256.update(chunk)
            sha512.update(chunk)
    return {"source_sha256": sha256.hexdigest(), "source_sha512": sha512.hexdigest()}


def copy_tree(src: pathlib.Path, dst: pathlib.Path) -> None:
    if dst.exists():
        shutil.rmtree(dst)
    shutil.copytree(src, dst)


def read_json(path: pathlib.Path) -> object:
    return json.loads(read_text(path))


def git_output(repo: pathlib.Path, *args: str) -> str:
    return subprocess.check_output(["git", "-C", str(repo), *args], text=True).strip()


def port_version_value(entry: object) -> int:
    if not isinstance(entry, dict):
        return -1
    value = entry.get("port-version", 0)
    if isinstance(value, int):
        return value
    if isinstance(value, str) and value.isdigit():
        return int(value)
    return -1


def update_vcpkg_portfile(portfile_path: pathlib.Path, version: str, sha512: str) -> None:
    text = read_text(portfile_path)
    text = re.sub(
        r"# SHA512 for the v[0-9]+\.[0-9]+\.[0-9]+ tagged source archive; update it when bumping the\n"
        r"\s*# port to a new release\.",
        f"# SHA512 for the v{version} tagged source archive; update it when bumping the\n"
        f"    # port to a new release.",
        text,
        count=1,
    )
    text, replaced = re.subn(
        r'set\(_limitless_source_sha512\s*\n\s*"[^"]+"\s*\n\s*\)',
        'set(_limitless_source_sha512\n'
        f'        "{sha512}"\n'
        "    )",
        text,
        count=1,
        flags=re.S,
    )
    if replaced != 1:
        fail(f"unable to update vcpkg checksum in {portfile_path}")
    write_text(portfile_path, text)


def cmd_hash(args: argparse.Namespace) -> None:
    version = normalize_version(args.version)
    url = source_url(version)
    metadata = {"version": version, "source_url": url}
    metadata.update(hash_url(url))
    json.dump(metadata, sys.stdout, indent=2)
    sys.stdout.write("\n")


def cmd_sync_conan_center(args: argparse.Namespace) -> None:
    version = normalize_version(args.version)
    target_repo = pathlib.Path(args.target_repo).resolve()
    recipe_root = target_repo / "recipes" / "limitless"
    template_root = ROOT / "packaging" / "conan-center-index" / "recipes" / "limitless" / "all"
    copy_tree(template_root, recipe_root / "all")
    write_text(
        recipe_root / "config.yml",
        'versions:\n'
        f'  "{version}":\n'
        "    folder: all\n",
    )
    write_text(
        recipe_root / "all" / "conandata.yml",
        "sources:\n"
        f'  "{version}":\n'
        f'    url: "{args.source_url}"\n'
        f'    sha256: "{args.source_sha256}"\n',
    )


def cmd_sync_vcpkg(args: argparse.Namespace) -> None:
    version = normalize_version(args.version)
    target_repo = pathlib.Path(args.target_repo).resolve()
    port_src = ROOT / "packaging" / "vcpkg" / "ports" / "limitless"
    port_dst = target_repo / "ports" / "limitless"
    copy_tree(port_src, port_dst)
    update_vcpkg_portfile(port_dst / "portfile.cmake", version, args.source_sha512)

    vcpkg_manifest_path = port_dst / "vcpkg.json"
    vcpkg_manifest = read_json(vcpkg_manifest_path)
    if not isinstance(vcpkg_manifest, dict):
        fail(f"unexpected vcpkg manifest structure: {vcpkg_manifest_path}")
    vcpkg_manifest["version-string"] = version
    write_json(vcpkg_manifest_path, vcpkg_manifest)

    git_output(target_repo, "add", "ports/limitless")
    git_tree = git_output(target_repo, "write-tree", "--prefix=ports/limitless")

    versions_path = target_repo / "versions" / "l-" / "limitless.json"
    if versions_path.exists():
        versions_data = read_json(versions_path)
    else:
        versions_data = {"versions": []}
    if not isinstance(versions_data, dict) or not isinstance(versions_data.get("versions"), list):
        fail(f"unexpected versions file structure: {versions_path}")
    versions = [
        entry
        for entry in versions_data["versions"]
        if not (
            isinstance(entry, dict)
            and entry.get("version") == version
            and port_version_value(entry) == 0
        )
    ]
    versions.insert(0, {"git-tree": git_tree, "version": version, "port-version": 0})
    versions_data["versions"] = versions
    write_json(versions_path, versions_data)

    baseline_path = target_repo / "versions" / "baseline.json"
    baseline_data = read_json(baseline_path)
    if not isinstance(baseline_data, dict):
        fail(f"unexpected baseline structure: {baseline_path}")
    default_registry = baseline_data.setdefault("default", {})
    if not isinstance(default_registry, dict):
        fail(f"unexpected default baseline structure: {baseline_path}")
    default_registry["limitless"] = {"baseline": version, "port-version": 0}
    write_json(baseline_path, baseline_data)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)

    hash_parser = subparsers.add_parser("hash")
    hash_parser.add_argument("--version", required=True)
    hash_parser.set_defaults(func=cmd_hash)

    conan_parser = subparsers.add_parser("sync-conan-center")
    conan_parser.add_argument("--target-repo", required=True)
    conan_parser.add_argument("--version", required=True)
    conan_parser.add_argument("--source-url", required=True)
    conan_parser.add_argument("--source-sha256", required=True)
    conan_parser.set_defaults(func=cmd_sync_conan_center)

    vcpkg_parser = subparsers.add_parser("sync-vcpkg")
    vcpkg_parser.add_argument("--target-repo", required=True)
    vcpkg_parser.add_argument("--version", required=True)
    vcpkg_parser.add_argument("--source-sha512", required=True)
    vcpkg_parser.set_defaults(func=cmd_sync_vcpkg)

    return parser


def main() -> None:
    parser = build_parser()
    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
