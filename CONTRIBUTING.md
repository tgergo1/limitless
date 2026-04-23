<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Contributing

## Ground rules

- Keep baseline compatibility: C99 and C++11.
- Keep `limitless.h` zero-include.
- Keep API changes additive unless explicitly planned as breaking.
- Keep all CI checks green before requesting review.

## Local checks

Run from repository root.

```sh
python3 tools/check_version.py
python3 tools/check_reserved_filenames.py

CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh default
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh limb64
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh noexceptions
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh asan-ubsan
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh lsan
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh tsan
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh valgrind

bash tests/ci/run_negative_compile.sh
bash tests/ci/run_clang_analyze.sh
bash tests/ci/run_clang_tidy.sh
bash tests/ci/run_cppcheck.sh
bash tests/ci/run_repo_lint.sh
bash tests/ci/check_spdx_headers.sh
bash tests/ci/run_fuzz_smoke.sh
bash tests/ci/run_bench_regression.sh
```

`m32`, `lsan`, `tsan`, and `valgrind` availability depends on host tooling support (fully covered in GitHub CI).

## Coverage check

```sh
python3 -m pip install gcovr
CC_BIN=gcc CXX_BIN=g++ LIMITLESS_DIFF_ITERS=800 bash tests/ci/run_coverage.sh
```

## Packaging validation

Recommended external publication targets are ConanCenter and the main vcpkg
registry.

```sh
cmake -S . -B build/local -DCMAKE_INSTALL_PREFIX=$PWD/build/local/install
cmake --install build/local

pkg-config --cflags --libs limitless

python3 -m pip install conan
conan profile detect --force
conan create . --version "$(cat LIMITLESS_VERSION.txt)"
```

To validate vcpkg overlay locally:

```sh
$VCPKG_ROOT/vcpkg install limitless --overlay-ports=$PWD/packaging/vcpkg/ports
```

The shipped vcpkg port is also structured so it can be submitted upstream and
fetch tagged release sources outside the repository tree.

Stable release tags also trigger GitHub Actions automation that opens or
updates the upstream ConanCenter and vcpkg pull requests. Configure a
`PACKAGE_REGISTRY_TOKEN` repository secret for a GitHub account that can fork
those upstream repositories to enable fully automated submissions.

## Regenerating test vectors

```sh
python3 tests/gen_vectors.py
```

Commit both generated files if they change:

- `tests/generated_core_vectors.inc`
- `tests/generated_cpp_vectors.inc`

## Differential check

```sh
cc -std=c99 -Wall -Wextra -Werror -pedantic tests/ci/limitless_cli.c -o /tmp/limitless_cli
bash tests/ci/run_differential.sh /tmp/limitless_cli 5000
```

## Pull request checklist

- Tests updated if behavior changed.
- Packaging metadata updated if install surface changed.
- README and changelog updated for user-visible changes.
- Version files/macros kept consistent (`python3 tools/check_version.py`).
- Namespace migration impact considered (`docs/MIGRATION_CPP_NAMESPACE.md`).
- Required status checks pass (`docs/RELEASE_CHECKLIST.md`).

## Branch protection recommendation

Protect `main` with required status checks from `.github/workflows/ci.yml` and require pull-request review before merge.
