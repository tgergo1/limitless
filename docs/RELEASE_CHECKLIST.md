<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Release Checklist

## Pre-tag

1. Run `python3 tools/check_version.py`.
2. Run `bash tests/ci/run_unix_matrix.sh default`.
3. Run `bash tests/ci/packaging_smoke.sh`.
4. Run `bash tests/ci/run_bench_regression.sh`.
5. Confirm changelog entry for release version.

## Required CI checks

- `ci-gate`
- `packaging-gate`
- `coverage-gate`
- `static-analysis-gate`
- `fuzz-gate`
- `security-gate`

## Tagging

1. Tag as `vX.Y.Z` or prerelease (`vX.Y.Z-rcN`, `vX.Y.Z-betaN`).
2. Push tag and verify release workflow completion.

## Artifact verification

1. Validate `SHA256SUMS`.
2. Validate SPDX SBOM.
3. Build and run C/C++ consumers from install archive.
4. Verify provenance attestation was generated.

## Rollback

- If critical regression is discovered, delete invalid release tag, open incident issue, and publish corrected tag after fixes.
