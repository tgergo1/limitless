<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog and this project follows Semantic Versioning.

## [Unreleased]

### Added
- Release-grade packaging metadata for CMake, pkg-config, Conan, and vcpkg overlay usage.
- Large multi-compiler CI matrix and tag-driven release automation with checksums, SPDX SBOM, and provenance attestation.
- Additional API completeness, aliasing, failure-atomicity, parser fuzz, thread-behavior, and differential reference tests.
- Extra CI modes/jobs for `m32`, `lsan`, `tsan`, musl Linux, and big-endian (`s390x`) emulation.

## [0.1.0] - 2026-02-14

### Added
- Initial public release of `limitless`.
- Exact arbitrary-precision integer/rational C API in `limitless.h`.
- C++ wrapper in `limitless.hpp` with operator overloads and parse/format helpers.
- Deterministic generated test vectors and stress/property test suites.
