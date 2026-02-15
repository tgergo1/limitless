#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]

# Case-insensitive guard for repo-root entries that could shadow standard headers
# when users compile with -I <repo-root>.
RESERVED = {
    # C headers (without .h)
    "assert",
    "complex",
    "ctype",
    "errno",
    "fenv",
    "float",
    "inttypes",
    "iso646",
    "limits",
    "locale",
    "math",
    "setjmp",
    "signal",
    "stdalign",
    "stdarg",
    "stdatomic",
    "stdbool",
    "stddef",
    "stdint",
    "stdio",
    "stdlib",
    "stdnoreturn",
    "string",
    "tgmath",
    "threads",
    "time",
    "uchar",
    "wchar",
    "wctype",
    # C++ headers
    "algorithm",
    "array",
    "atomic",
    "bit",
    "chrono",
    "codecvt",
    "compare",
    "concepts",
    "condition_variable",
    "coroutine",
    "deque",
    "exception",
    "filesystem",
    "format",
    "forward_list",
    "fstream",
    "functional",
    "future",
    "initializer_list",
    "iomanip",
    "ios",
    "iosfwd",
    "iostream",
    "istream",
    "iterator",
    "latch",
    "list",
    "map",
    "memory",
    "memory_resource",
    "mutex",
    "new",
    "numbers",
    "numeric",
    "optional",
    "ostream",
    "queue",
    "random",
    "ranges",
    "ratio",
    "regex",
    "scoped_allocator",
    "semaphore",
    "set",
    "shared_mutex",
    "span",
    "sstream",
    "stack",
    "stdexcept",
    "stop_token",
    "streambuf",
    "string_view",
    "strstream",
    "system_error",
    "thread",
    "tuple",
    "type_traits",
    "typeindex",
    "typeinfo",
    "unordered_map",
    "unordered_set",
    "utility",
    "valarray",
    "variant",
    "vector",
    "version",
}


def main() -> int:
    collisions: list[str] = []
    for entry in ROOT.iterdir():
      name = entry.name
      if name in {".", "..", ".git"}:
          continue
      if name.lower() in RESERVED:
          collisions.append(name)

    if collisions:
        print(
            "reserved filename collision at repo root (case-insensitive): "
            + ", ".join(sorted(collisions)),
            file=sys.stderr,
        )
        return 1

    print("reserved filename check ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
