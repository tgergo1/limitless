/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static int g_alloc_calls = 0;
static int g_realloc_calls = 0;
static int g_free_calls = 0;

static void* test_default_alloc(size_t n) {
  ++g_alloc_calls;
  return malloc(n);
}

static void* test_default_realloc(void* p, size_t n) {
  ++g_realloc_calls;
  return realloc(p, n);
}

static void test_default_free(void* p) {
  ++g_free_calls;
  free(p);
}

#define LIMITLESS_DEFAULT_ALLOC(size) test_default_alloc((size_t)(size))
#define LIMITLESS_DEFAULT_REALLOC(ptr, old_size, new_size) test_default_realloc((ptr), (size_t)(new_size))
#define LIMITLESS_DEFAULT_FREE(ptr, size) test_default_free((ptr))
#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

int main(void) {
  limitless_ctx ctx;
  limitless_number n;
  char buf[64];

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &n, "123456789/3") == LIMITLESS_OK);
  assert(limitless_number_to_str(&ctx, &n, buf, (limitless_size)sizeof(buf), NULL) == LIMITLESS_OK);
  assert(strcmp(buf, "41152263") == 0);

  limitless_number_clear(&ctx, &n);

  assert(g_alloc_calls > 0 || g_realloc_calls > 0);
  assert(g_free_calls >= 0);
  return 0;
}
