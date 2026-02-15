<!-- SPDX-License-Identifier: GPL-3.0-only -->
# C++ Namespace Migration

`limitless.hpp` now supports a strict namespace-first mode.

## Modes

- Default bridge mode: `LIMITLESS_CPP_LEGACY_API=1` (default)
  - Global wrappers remain available (`limitless_cpp_*`).
  - Preferred API is namespaced (`limitless::number`, `limitless::limitless_cpp_*`).
- Strict mode: `LIMITLESS_CPP_LEGACY_API=0`
  - Global wrappers are removed at compile time.
  - Only namespaced API is available.

## Deprecation policy

- Legacy global wrappers are deprecated in bridge mode.
- Planned removal target for legacy globals: `v0.3.0`.

## Migration steps

1. Replace `limitless_number` with `limitless::number` in C++ code.
2. Replace global wrapper calls with namespaced equivalents.
3. Enable strict mode in CI: `#define LIMITLESS_CPP_LEGACY_API 0` before including `limitless.hpp`.
