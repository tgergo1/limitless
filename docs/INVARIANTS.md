<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Invariants

## Numeric representation

- Integer zero is canonical (`sign=0`, `used=0`).
- Rational denominator is strictly positive.
- Rational values are normalized (`gcd(num, den)=1`).
- Rational denominator `1` collapses to integer representation.

## API behavior

- Operations are failure-atomic: output remains unchanged on failure.
- Divide-by-zero reports `LIMITLESS_EDIVZERO`.
- Integer-only operations reject non-integers with `LIMITLESS_ETYPE`.
- Conversion APIs return explicit status (`ERANGE`, `ETYPE`, etc.) without UB.

## C++ wrapper state

- Default context override and last-status are thread-local and TU-coherent.
- Strict mode can disable legacy global wrappers.
