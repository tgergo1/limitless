<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Limitless

Single-header exact big numbers for c and c++.

it stores exact integers and exact fractions (`a/b`) and grows until memory runs out.

## What is in this repo

- `limitless.h`: C API + implementation (`#define LIMITLESS_IMPLEMENTATION` in one TU)
- `limitless.hpp`: C++ wrapper with operators and RAII on top of the C API
- `tests/`: baseline tests, generated matrix tests, stress tests, CI wrappers
  - API completeness + alias/failure-atomic tests
  - parser fuzz + valid roundtrip stress
  - C++ thread behavior tests
  - differential checks against a python `fractions.Fraction` reference
- packaging metadata for CMake, pkg-config, Conan, and vcpkg overlay port

current version is tracked in `VERSION` and exposed in `limitless.h`:

- `LIMITLESS_VERSION_MAJOR`
- `LIMITLESS_VERSION_MINOR`
- `LIMITLESS_VERSION_PATCH`
- `LIMITLESS_VERSION_STRING`

## Minimal use

### C

```c
#define LIMITLESS_IMPLEMENTATION
#include "limitless.h"

int main(void) {
  limitless_ctx ctx;
  limitless_number x, y;
  char buf[64];

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &x) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &y) != LIMITLESS_OK) return 1;

  limitless_number_from_str(&ctx, &x, "7");
  limitless_number_from_str(&ctx, &y, "3");
  limitless_number_div(&ctx, &x, &x, &y);
  limitless_number_to_str(&ctx, &x, buf, sizeof(buf), NULL);

  limitless_number_clear(&ctx, &x);
  limitless_number_clear(&ctx, &y);
  return 0;
}
```

### C++

```cpp
#define LIMITLESS_IMPLEMENTATION
#include "limitless.hpp"

int main() {
  int a = 3;
  limitless_number x = 33424234;
  limitless_number y = (x + a) / 2.3f;
  return y.str() == "140191410946048/9646899" ? 0 : 1;
}
```

## Package consumption

### CMake

project side:

```cmake
find_package(Limitless CONFIG REQUIRED)
add_executable(app main.c)
target_link_libraries(app PRIVATE limitless::limitless)
```

build example:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/limitless/install
cmake --build build
```

### pkg-config

```sh
cc main.c $(pkg-config --cflags --libs limitless) -o app
```

### Conan 2

```sh
conan profile detect --force
conan create . --version "$(cat VERSION)"
```

consumer `conanfile.txt` example:

```ini
[requires]
limitless/0.1.0

[generators]
CMakeDeps
CMakeToolchain
```

### vcpkg overlay port

```sh
$VCPKG_ROOT/vcpkg install limitless --overlay-ports=$PWD/packaging/vcpkg/ports
```

## Local build and test

quick local matrix on unix-like hosts:

```sh
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh default
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh limb64
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh noexceptions
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh m32
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh asan-ubsan
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh lsan
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh tsan
CC_BIN=cc CXX_BIN=c++ bash tests/ci/run_unix_matrix.sh extended-stress
```

`m32` and some sanitizer modes are linux-toolchain dependent.

packaging smoke checks:

```sh
python3 -m pip install conan
bash tests/ci/packaging_smoke.sh
```

regenerate committed vectors:

```sh
python3 tests/gen_vectors.py
```

run standalone differential checks:

```sh
cc -std=c99 -Wall -Wextra -Werror -pedantic tests/ci/limitless_cli.c -o build/limitless_cli
bash tests/ci/run_differential.sh build/limitless_cli 5000
```

## Release model

- tags drive releases: `vX.Y.Z`
- release workflow validates tag/version consistency first
- release assets include:
  - source tar/zip
  - install-layout archive
  - `SHA256SUMS`
  - SPDX SBOM
  - GitHub provenance attestation

## License

GPL-3.0-only. see `LICENSE`.
