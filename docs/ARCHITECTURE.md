<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Architecture

## Core model

- Single-header C core in `limitless.h`.
- Runtime numeric model: tagged union of bigint integer or normalized rational.
- C++ wrapper in `limitless.hpp` provides RAII and operators over C API primitives.

## Memory and context

- Allocation is context-scoped (`limitless_ctx`).
- No hidden mutable global allocator state.
- Failure-atomic APIs use compute-and-swap style semantics.

## Major layers

1. Bigint primitives (limbs, reserve/copy/norm, arithmetic).
2. Rational normalization and reduction.
3. Public C APIs for parse/format/ops/conversions.
4. C++ ergonomic wrapper and thread-local default-context/status helpers.

## Build and packaging

- Header-only distribution model.
- CMake target, pkg-config metadata, Conan recipe, and vcpkg overlay port.
- CI matrix covers major OS/compiler/mode combinations.
