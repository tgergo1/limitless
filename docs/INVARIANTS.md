<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Invariants

## Numeric representation

- Integer zero is canonical (`sign=0`, `used=0`).
- Rational denominator is strictly positive.
- Rational values are normalized (`gcd(num, den)=1`).
- Rational denominator `1` collapses to integer representation.
- Canonical binary output uses big-endian, non-zero, minimally encoded
  magnitudes. Rationals are reduced and never encode denominator `1`.

## API behavior

- Operations are failure-atomic: output remains unchanged on failure.
- Divide-by-zero reports `LIMITLESS_EDIVZERO`.
- Integer-only operations reject non-integers with `LIMITLESS_ETYPE`.
- Conversion APIs return explicit status (`ERANGE`, `ETYPE`, etc.) without UB.
- Integer `divmod` truncates toward zero and gives the remainder the dividend's
  sign. Modular inverse normalizes its successful result to `[0, modulus)`.
- Bitwise operators use unbounded two's-complement semantics; logical shifts
  and bit inspection use the sign-magnitude magnitude.
- Decimal and scientific input is converted directly to exact normalized
  integers or rationals. Floating export rounds nearest with ties to even.

## C++ wrapper state

- Default context override and last-status are thread-local and TU-coherent.
- Strict mode can disable legacy global wrappers.
- Stream extraction only commits a parsed value on success and reports parse
  or allocation failure through both the stream state and last-status.
