<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Changelog

All notable changes to this project will be documented in this file.

The format is based on Keep a Changelog and this project follows Semantic Versioning.

## [Unreleased]

## [0.3.0] - 2026-09-03

### Added
- Complete numeric API support across integer, rational, conversion, and comparison operations.
- Expanded benchmark reporting, regression coverage, and optimization work for small-modulus and arithmetic hot paths.
- Improved CI gating around coverage, static analysis, and security validation for release readiness.
- Packaging and release automation refinements for install artifacts, hash verification, and upstream registry integration.

## [0.2.0] - 2026-04-22

### Added
- Release-grade packaging metadata for CMake, pkg-config, Conan, and vcpkg overlay usage.
- Large multi-compiler CI matrix and tag-driven release automation with checksums, SPDX SBOM, and provenance attestation.
- Additional API completeness, aliasing, failure-atomicity, parser fuzz, thread-behavior, and differential reference tests.
- Extra CI modes/jobs for `m32`, `lsan`, `tsan`, musl Linux, and big-endian (`s390x`) emulation.
- C++ namespace migration controls (`LIMITLESS_CPP_LEGACY_API`) with strict-mode and negative-compile regression tests.
- Additional parse/conversion/invariant/allocator-contract correctness suites and expanded differential-oracle operations.
- Coverage-guided fuzz harnesses (`fuzz_parse`, `fuzz_arith`, `fuzz_format`) plus required fuzz-smoke workflow.
- Expanded release/archive verification for both C and C++ consumers and repo-root reserved filename collision checks.
- Broader static analysis and lint scripts (`clang-tidy`, `cppcheck`, `actionlint`, `shellcheck`, `yamllint`, SPDX header checks).
- Benchmark regression framework and CI benchmark gate.
- Governance additions: issue templates, code of conduct, support/maintainers docs, architecture/invariant/thread-safety/release checklist docs.

## [0.1.0] - 2026-02-14

### Added
- Initial public release of `limitless`.
- Exact arbitrary-precision integer/rational C API in `limitless.h`.
- C++ wrapper in `limitless.hpp` with operator overloads and parse/format helpers.
- Deterministic generated test vectors and stress/property test suites.
