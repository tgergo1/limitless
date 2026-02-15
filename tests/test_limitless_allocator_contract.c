/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

typedef struct alloc_stats {
  limitless_size alloc_calls;
  limitless_size realloc_calls;
  limitless_size free_calls;
  limitless_size live_bytes;
  limitless_size peak_live_bytes;
} alloc_stats;

static void update_peak(alloc_stats* s) {
  if (s->live_bytes > s->peak_live_bytes) {
    s->peak_live_bytes = s->live_bytes;
  }
}

static void* stats_alloc(void* user, limitless_size size) {
  alloc_stats* s = (alloc_stats*)user;
  ++s->alloc_calls;
  s->live_bytes += size;
  update_peak(s);
  return malloc((size_t)size);
}

static void* stats_realloc(void* user, void* ptr, limitless_size old_size, limitless_size new_size) {
  alloc_stats* s = (alloc_stats*)user;
  ++s->realloc_calls;
  if (s->live_bytes >= old_size) {
    s->live_bytes -= old_size;
  } else {
    s->live_bytes = 0;
  }
  s->live_bytes += new_size;
  update_peak(s);
  return realloc(ptr, (size_t)new_size);
}

static void stats_free(void* user, void* ptr, limitless_size size) {
  alloc_stats* s = (alloc_stats*)user;
  ++s->free_calls;
  if (s->live_bytes >= size) {
    s->live_bytes -= size;
  } else {
    s->live_bytes = 0;
  }
  free(ptr);
}

int main(void) {
  limitless_status st;
  alloc_stats stats;
  limitless_alloc alloc;
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  char text[2048];

  memset(&stats, 0, sizeof(stats));
  alloc.alloc = stats_alloc;
  alloc.realloc = stats_realloc;
  alloc.free = stats_free;
  alloc.user = &stats;

  st = limitless_ctx_init(&ctx, &alloc);
  assert(st == LIMITLESS_OK);
  st = limitless_number_init(&ctx, &a);
  assert(st == LIMITLESS_OK);
  st = limitless_number_init(&ctx, &b);
  assert(st == LIMITLESS_OK);
  st = limitless_number_init(&ctx, &out);
  assert(st == LIMITLESS_OK);

  assert(limitless_number_from_str(&ctx, &a, "12345678901234567890123456789012345678901234567890/97") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &b, "99887766554433221100998877665544332211009988776655/89") == LIMITLESS_OK);
  assert(limitless_number_mul(&ctx, &out, &a, &b) == LIMITLESS_OK);
  assert(limitless_number_to_str(&ctx, &out, text, (limitless_size)sizeof(text), NULL) == LIMITLESS_OK);
  assert(text[0] != '\0');

  assert(stats.alloc_calls > 0 || stats.realloc_calls > 0);
  assert(stats.peak_live_bytes > 0);

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);

  assert(stats.free_calls > 0);
  assert(stats.live_bytes == 0);

  printf("allocator contract tests ok\n");
  return 0;
}
