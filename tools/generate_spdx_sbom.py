#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import argparse
import datetime as dt
import hashlib
import json
import pathlib
import re

SPDX_ID_RE = re.compile(r"[^A-Za-z0-9.-]")


def spdx_id(name: str) -> str:
    return SPDX_ID_RE.sub("-", name)


def sha256(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def relpath(path: pathlib.Path, root: pathlib.Path) -> str:
    return path.relative_to(root).as_posix()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True)
    parser.add_argument("--name", required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--out", required=True)
    args = parser.parse_args()

    root = pathlib.Path(args.root).resolve()
    out = pathlib.Path(args.out).resolve()
    created = dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")

    files = []
    relationships = []

    package_spdx_id = f"SPDXRef-Package-{spdx_id(args.name)}"

    for path in sorted(p for p in root.rglob("*") if p.is_file()):
        if ".git" in path.parts:
            continue
        if path == out:
            continue
        rel = relpath(path, root)
        fid = f"SPDXRef-File-{spdx_id(rel)}"
        files.append(
            {
                "SPDXID": fid,
                "fileName": rel,
                "checksums": [{"algorithm": "SHA256", "checksumValue": sha256(path)}],
                "licenseConcluded": "GPL-3.0-only",
                "licenseInfoInFiles": ["GPL-3.0-only"],
                "copyrightText": "NOASSERTION",
            }
        )
        relationships.append(
            {
                "spdxElementId": package_spdx_id,
                "relationshipType": "CONTAINS",
                "relatedSpdxElement": fid,
            }
        )

    doc = {
        "spdxVersion": "SPDX-2.3",
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": f"{args.name}-{args.version}",
        "documentNamespace": f"https://github.com/tgergo1/limitless/spdx/{args.version}",
        "creationInfo": {
            "created": created,
            "creators": ["Tool: generate_spdx_sbom.py"],
            "licenseListVersion": "3.25",
        },
        "packages": [
            {
                "name": args.name,
                "SPDXID": package_spdx_id,
                "versionInfo": args.version,
                "downloadLocation": "NOASSERTION",
                "filesAnalyzed": True,
                "licenseConcluded": "GPL-3.0-only",
                "licenseDeclared": "GPL-3.0-only",
                "copyrightText": "NOASSERTION",
            }
        ],
        "files": files,
        "relationships": relationships,
    }

    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(doc, indent=2, sort_keys=False) + "\n", encoding="utf-8")
    print(f"wrote {out}")


if __name__ == "__main__":
    main()
