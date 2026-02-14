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

CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh default
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh limb64
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh noexceptions
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh asan-ubsan
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh lsan
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh tsan
```

`m32`, `lsan`, and `tsan` availability depends on host toolchain support (fully covered in GitHub CI).

## Packaging validation

```sh
cmake -S . -B build/local -DCMAKE_INSTALL_PREFIX=$PWD/build/local/install
cmake --install build/local

pkg-config --cflags --libs limitless

python3 -m pip install conan
conan profile detect --force
conan create . --version "$(cat VERSION)"
```

To validate vcpkg overlay locally:

```sh
$VCPKG_ROOT/vcpkg install limitless --overlay-ports=$PWD/packaging/vcpkg/ports
```

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

## Branch protection recommendation

Protect `main` with required status checks from `.github/workflows/ci.yml` and require pull-request review before merge.
