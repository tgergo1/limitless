#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import argparse
import json
import pathlib
import sys


def fail(msg: str) -> None:
    print(f"spdx validation error: {msg}", file=sys.stderr)
    raise SystemExit(1)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("sbom", help="path to SPDX JSON")
    args = ap.parse_args()

    path = pathlib.Path(args.sbom)
    if not path.is_file():
        fail(f"file not found: {path}")

    doc = json.loads(path.read_text(encoding="utf-8"))

    for key in ["spdxVersion", "SPDXID", "name", "creationInfo", "packages", "files", "relationships"]:
        if key not in doc:
            fail(f"missing top-level key: {key}")

    if doc["spdxVersion"] != "SPDX-2.3":
        fail(f"unexpected spdxVersion: {doc['spdxVersion']}")

    if not isinstance(doc["packages"], list) or not doc["packages"]:
        fail("packages list must be non-empty")
    if not isinstance(doc["files"], list) or not doc["files"]:
        fail("files list must be non-empty")

    package = doc["packages"][0]
    for key in ["name", "SPDXID", "versionInfo", "licenseDeclared", "licenseConcluded"]:
        if key not in package:
            fail(f"package missing key: {key}")

    package_id = package["SPDXID"]

    for idx, fobj in enumerate(doc["files"]):
        if "SPDXID" not in fobj or "fileName" not in fobj:
            fail(f"file #{idx} missing SPDXID/fileName")
        checksums = fobj.get("checksums")
        if not isinstance(checksums, list) or not checksums:
            fail(f"file #{idx} missing checksums")
        if not any(c.get("algorithm") == "SHA256" and c.get("checksumValue") for c in checksums):
            fail(f"file #{idx} missing SHA256 checksum")

    rels = doc["relationships"]
    if not isinstance(rels, list) or not rels:
        fail("relationships list must be non-empty")

    contains = [
        r for r in rels
        if r.get("spdxElementId") == package_id and r.get("relationshipType") == "CONTAINS"
    ]
    if not contains:
        fail("missing package CONTAINS relationships")

    print(f"spdx validation ok: {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
