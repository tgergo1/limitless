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


def frac_to_str(v: Fraction) -> str:
    if v.denominator == 1:
        return str(v.numerator)
    return f"{v.numerator}/{v.denominator}"


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


def expect_ok(cli: str, op: str, a: str, b: str | None, expected: str) -> None:
    args = [op, a] if b is None else [op, a, b]
    ok, payload = run_cli(cli, *args)
    if not ok:
        raise AssertionError(f"{op}({a},{b}) failed with {payload}, expected OK:{expected}")
    if payload != expected:
        raise AssertionError(f"{op}({a},{b}) -> {payload}, expected {expected}")


def expect_err(cli: str, op: str, a: str, b: str | None, status: int) -> None:
    args = [op, a] if b is None else [op, a, b]
    ok, payload = run_cli(cli, *args)
    if ok:
        raise AssertionError(f"{op}({a},{b}) unexpectedly succeeded: {payload}")
    if payload != status:
        raise AssertionError(f"{op}({a},{b}) status {payload}, expected {status}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--cli", required=True, help="path to compiled limitless_cli")
    ap.add_argument("--iters", type=int, default=3000)
    ap.add_argument("--seed", type=int, default=0xC0FFEE)
    args = ap.parse_args()

    rng = random.Random(args.seed)

    for _ in range(args.iters):
        a_s = random_fraction_literal(rng)
        b_s = random_fraction_literal(rng)

        a = Fraction(a_s)
        b = Fraction(b_s)

        expect_ok(args.cli, "parse", a_s, None, frac_to_str(a))
        expect_ok(args.cli, "add", a_s, b_s, frac_to_str(a + b))
        expect_ok(args.cli, "sub", a_s, b_s, frac_to_str(a - b))
        expect_ok(args.cli, "mul", a_s, b_s, frac_to_str(a * b))
        expect_ok(args.cli, "neg", a_s, None, frac_to_str(-a))
        expect_ok(args.cli, "abs", a_s, None, frac_to_str(abs(a)))

        cmp_expected = -1 if a < b else (1 if a > b else 0)
        expect_ok(args.cli, "cmp", a_s, b_s, str(cmp_expected))

        if b == 0:
            expect_err(args.cli, "div", a_s, b_s, LIMITLESS_EDIVZERO)
        else:
            expect_ok(args.cli, "div", a_s, b_s, frac_to_str(a / b))

        if a.denominator == 1 and b.denominator == 1:
            ai = abs(a.numerator)
            bi = abs(b.numerator)
            expect_ok(args.cli, "gcd", a_s, b_s, str(math.gcd(ai, bi)))

    print(f"differential checks passed ({args.iters} iterations)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
