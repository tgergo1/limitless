#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
from __future__ import annotations

import argparse
import math
import random
import subprocess
import sys
from fractions import Fraction

LIMITLESS_OK = 0
LIMITLESS_EDIVZERO = 3
LIMITLESS_ERANGE = 5
LIMITLESS_ETYPE = 6

MIN_I64 = -(1 << 63)
MAX_I64 = (1 << 63) - 1
MAX_U64 = (1 << 64) - 1


def frac_to_str(v: Fraction) -> str:
    if v.denominator == 1:
        return str(v.numerator)
    return f"{v.numerator}/{v.denominator}"


def int_to_base(n: int, base: int) -> str:
    assert 2 <= base <= 36
    if n == 0:
        return "0"
    digits = "0123456789abcdefghijklmnopqrstuvwxyz"
    sign = "-" if n < 0 else ""
    x = abs(n)
    out: list[str] = []
    while x > 0:
        out.append(digits[x % base])
        x //= base
    return sign + "".join(reversed(out))


def frac_to_base_str(v: Fraction, base: int) -> str:
    if v.denominator == 1:
        return int_to_base(v.numerator, base)
    return f"{int_to_base(v.numerator, base)}/{int_to_base(v.denominator, base)}"


def random_int(rng: random.Random, max_digits: int, positive_only: bool = False) -> int:
    if rng.randrange(9) == 0:
        return 0
    digits = rng.randrange(1, max_digits + 1)
    first = rng.randrange(1, 10)
    rest = "".join(str(rng.randrange(10)) for _ in range(max(0, digits - 1)))
    n = int(str(first) + rest)
    if not positive_only and rng.randrange(2) == 0:
        n = -n
    return n


def random_fraction_literal(rng: random.Random) -> str:
    num = random_int(rng, 70, positive_only=False)
    if rng.randrange(3) != 0:
        return str(num)
    den = random_int(rng, 40, positive_only=True)
    if den == 0:
        den = 1
    return f"{num}/{den}"


def run_cli(cli: str, *args: str) -> tuple[bool, str | int]:
    cp = subprocess.run([cli, *args], check=False, capture_output=True, text=True)
    out = cp.stdout.strip()
    if out.startswith("OK:"):
        return (True, out[3:])
    if out.startswith("ERR:"):
        try:
            return (False, int(out[4:]))
        except ValueError:
            pass
    raise RuntimeError(f"unexpected CLI output for {args}: stdout={cp.stdout!r} stderr={cp.stderr!r}")


def expect_ok(cli: str, op: str, args: list[str], expected: str) -> None:
    ok, payload = run_cli(cli, op, *args)
    if not ok:
        raise AssertionError(f"{op}{tuple(args)} failed with {payload}, expected OK:{expected}")
    if payload != expected:
        raise AssertionError(f"{op}{tuple(args)} -> {payload}, expected {expected}")


def expect_err(cli: str, op: str, args: list[str], status: int) -> None:
    ok, payload = run_cli(cli, op, *args)
    if ok:
        raise AssertionError(f"{op}{tuple(args)} unexpectedly succeeded: {payload}")
    if payload != status:
        raise AssertionError(f"{op}{tuple(args)} status {payload}, expected {status}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--cli", required=True, help="path to compiled limitless_cli")
    ap.add_argument("--iters", type=int, default=3000)
    ap.add_argument("--seed", type=int, default=0xC0FFEE)
    args = ap.parse_args()

    rng = random.Random(args.seed)

    try:
        for _ in range(args.iters):
            a_s = random_fraction_literal(rng)
            b_s = random_fraction_literal(rng)
            base = rng.choice([2, 8, 10, 16, 36])
            exp = rng.randrange(0, 33)

            a = Fraction(a_s)
            b = Fraction(b_s)

            expect_ok(args.cli, "parse", [a_s], frac_to_str(a))
            expect_ok(args.cli, "add", [a_s, b_s], frac_to_str(a + b))
            expect_ok(args.cli, "sub", [a_s, b_s], frac_to_str(a - b))
            expect_ok(args.cli, "mul", [a_s, b_s], frac_to_str(a * b))
            expect_ok(args.cli, "neg", [a_s], frac_to_str(-a))
            expect_ok(args.cli, "abs", [a_s], frac_to_str(abs(a)))

            cmp_expected = -1 if a < b else (1 if a > b else 0)
            expect_ok(args.cli, "cmp", [a_s, b_s], str(cmp_expected))

            if b == 0:
                expect_err(args.cli, "div", [a_s, b_s], LIMITLESS_EDIVZERO)
            else:
                expect_ok(args.cli, "div", [a_s, b_s], frac_to_str(a / b))

            if a.denominator == 1 and b.denominator == 1:
                ai = a.numerator
                bi = b.numerator
                expect_ok(args.cli, "gcd", [a_s, b_s], str(math.gcd(abs(ai), abs(bi))))
            else:
                expect_err(args.cli, "gcd", [a_s, b_s], LIMITLESS_ETYPE)

            if a.denominator == 1 and abs(a.numerator) <= 10**6:
                ai = a.numerator
                expect_ok(args.cli, "pow", [a_s, str(exp)], str(pow(ai, exp)))
            elif a.denominator == 1:
                # Keep differential runtime bounded and avoid enormous exponents on huge integers.
                pass
            else:
                expect_err(args.cli, "pow", [a_s, str(exp)], LIMITLESS_ETYPE)

            if a.denominator != 1:
                expect_err(args.cli, "modexp", [a_s, str(exp), b_s], LIMITLESS_ETYPE)
            elif b.denominator != 1:
                expect_err(args.cli, "modexp", [a_s, str(exp), b_s], LIMITLESS_ETYPE)
            else:
                bi = b.numerator
                if bi <= 0:
                    expect_err(args.cli, "modexp", [a_s, str(exp), b_s], LIMITLESS_EDIVZERO)
                elif abs(a.numerator) > 10**9 or bi > 10**9:
                    # Keep differential runtime bounded for modular exponentiation.
                    pass
                else:
                    expect_ok(args.cli, "modexp", [a_s, str(exp), b_s], str(pow(a.numerator, exp, bi)))

            if a.denominator != 1:
                expect_err(args.cli, "to_i64", [a_s], LIMITLESS_ETYPE)
                expect_err(args.cli, "to_u64", [a_s], LIMITLESS_ETYPE)
            else:
                ai = a.numerator
                if MIN_I64 <= ai <= MAX_I64:
                    expect_ok(args.cli, "to_i64", [a_s], str(ai))
                else:
                    expect_err(args.cli, "to_i64", [a_s], LIMITLESS_ERANGE)

                if 0 <= ai <= MAX_U64:
                    expect_ok(args.cli, "to_u64", [a_s], str(ai))
                else:
                    expect_err(args.cli, "to_u64", [a_s], LIMITLESS_ERANGE)

            fmt_expected = frac_to_base_str(a, base)
            expect_ok(args.cli, "fmt", [a_s, str(base)], fmt_expected)
            expect_ok(args.cli, "parse_base", [fmt_expected, str(base)], frac_to_str(a))

        print(f"differential checks passed ({args.iters} iterations, seed={args.seed})")
        return 0
    except Exception as exc:
        print(f"differential failure (seed={args.seed}): {exc}", file=sys.stderr)
        raise


if __name__ == "__main__":
    sys.exit(main())
