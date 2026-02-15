<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Thread Safety

## C API

- `limitless` does not use hidden global mutable allocator state.
- A `limitless_ctx` is caller-owned. Thread safety is guaranteed when each thread owns its own context.
- Sharing one context across threads without external synchronization is not supported.
- Distinct `limitless_number` objects can be used concurrently if each thread operates on objects bound to its own context.

## C++ Wrapper

- Wrapper default context override and last-status are `thread_local` and translation-unit coherent.
- Each thread should set its own default context via `limitless::limitless_cpp_set_default_ctx`.
- Wrapper instances are not synchronized objects; cross-thread access requires caller locking.

## Recommended Pattern

1. Create one context per thread.
2. Initialize numbers in that context.
3. Avoid sharing mutable number objects across threads.
4. Use TSAN mode in CI (`run_unix_matrix.sh tsan`) for regression detection.
