/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef LIMITLESS_H
#define LIMITLESS_H

/*
Usage (C):
  #define LIMITLESS_IMPLEMENTATION
  #include "limitless.h"

Usage (C++):
  #include "limitless.hpp"
*/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LIMITLESS_API
#define LIMITLESS_API
#endif

#define LIMITLESS_VERSION_MAJOR 0
#define LIMITLESS_VERSION_MINOR 1
#define LIMITLESS_VERSION_PATCH 0
#define LIMITLESS_VERSION_STRING "0.1.0"

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

typedef unsigned char limitless_u8;
typedef signed int limitless_i32;
typedef unsigned int limitless_u32;
typedef signed long long limitless_i64;
typedef unsigned long long limitless_u64;
#if defined(__SIZE_TYPE__)
typedef __SIZE_TYPE__ limitless_size;
#elif defined(_MSC_VER) && defined(_WIN64)
typedef unsigned __int64 limitless_size;
#elif defined(_MSC_VER)
typedef unsigned int limitless_size;
#else
typedef unsigned long limitless_size;
#endif

typedef char limitless__assert_u8[(sizeof(limitless_u8) == 1) ? 1 : -1];
typedef char limitless__assert_i32[(sizeof(limitless_i32) == 4) ? 1 : -1];
typedef char limitless__assert_u32[(sizeof(limitless_u32) == 4) ? 1 : -1];
typedef char limitless__assert_i64[(sizeof(limitless_i64) == 8) ? 1 : -1];
typedef char limitless__assert_u64[(sizeof(limitless_u64) == 8) ? 1 : -1];
typedef char limitless__assert_size[(sizeof(limitless_size) >= sizeof(void*)) ? 1 : -1];

#define LIMITLESS_LIMB_BITS_32 32
#define LIMITLESS_LIMB_BITS_64 64

#ifndef LIMITLESS_LIMB_BITS
#define LIMITLESS_LIMB_BITS LIMITLESS_LIMB_BITS_32
#endif

#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
typedef limitless_u32 limitless_limb;
typedef limitless_u64 limitless_dlimb;
#elif (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_64)
typedef limitless_u64 limitless_limb;
#if defined(__SIZEOF_INT128__)
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
typedef unsigned __int128 limitless_dlimb;
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
#else
#error "LIMITLESS_LIMB_BITS=64 requires compiler support for unsigned __int128"
#endif
#else
#error "LIMITLESS_LIMB_BITS must be 32 or 64"
#endif

typedef enum limitless_status {
  LIMITLESS_OK = 0,
  LIMITLESS_EOOM = 1,
  LIMITLESS_EINVAL = 2,
  LIMITLESS_EDIVZERO = 3,
  LIMITLESS_EPARSE = 4,
  LIMITLESS_ERANGE = 5,
  LIMITLESS_ETYPE = 6,
  LIMITLESS_EBUF = 7
} limitless_status;

typedef enum limitless_kind {
  LIMITLESS_KIND_INT = 0,
  LIMITLESS_KIND_RAT = 1
} limitless_kind;

typedef void* (*limitless_alloc_fn)(void* user, limitless_size size);
typedef void* (*limitless_realloc_fn)(void* user, void* ptr, limitless_size old_size, limitless_size new_size);
typedef void (*limitless_free_fn)(void* user, void* ptr, limitless_size size);

typedef struct limitless_alloc {
  limitless_alloc_fn alloc;
  limitless_realloc_fn realloc;
  limitless_free_fn free;
  void* user;
} limitless_alloc;

typedef struct limitless_ctx {
  limitless_alloc allocator;
  limitless_size karatsuba_threshold;
} limitless_ctx;

typedef struct limitless_bigint {
  int sign;
  limitless_size used;
  limitless_size cap;
  limitless_limb* limbs;
} limitless_bigint;

typedef struct limitless_rational {
  limitless_bigint num;
  limitless_bigint den;
} limitless_rational;

typedef union limitless_value {
  limitless_bigint i;
  limitless_rational r;
} limitless_value;

typedef struct limitless_number {
  limitless_kind kind;
  limitless_value v;
} limitless_number;

LIMITLESS_API limitless_status limitless_ctx_init(limitless_ctx* ctx, const limitless_alloc* alloc);
LIMITLESS_API limitless_status limitless_ctx_init_default(limitless_ctx* ctx);
LIMITLESS_API void limitless_ctx_set_karatsuba_threshold(limitless_ctx* ctx, limitless_size limbs);

LIMITLESS_API limitless_status limitless_number_init(limitless_ctx* ctx, limitless_number* n);
LIMITLESS_API void limitless_number_clear(limitless_ctx* ctx, limitless_number* n);
LIMITLESS_API limitless_status limitless_number_copy(limitless_ctx* ctx, limitless_number* dst, const limitless_number* src);

LIMITLESS_API limitless_status limitless_number_from_i64(limitless_ctx* ctx, limitless_number* out, limitless_i64 v);
LIMITLESS_API limitless_status limitless_number_from_u64(limitless_ctx* ctx, limitless_number* out, limitless_u64 v);
LIMITLESS_API limitless_status limitless_number_from_long(limitless_ctx* ctx, limitless_number* out, long v);
LIMITLESS_API limitless_status limitless_number_from_ulong(limitless_ctx* ctx, limitless_number* out, unsigned long v);
LIMITLESS_API limitless_status limitless_number_from_ll(limitless_ctx* ctx, limitless_number* out, long long v);
LIMITLESS_API limitless_status limitless_number_from_ull(limitless_ctx* ctx, limitless_number* out, unsigned long long v);
LIMITLESS_API limitless_status limitless_number_from_float_exact(limitless_ctx* ctx, limitless_number* out, float v);
LIMITLESS_API limitless_status limitless_number_from_double_exact(limitless_ctx* ctx, limitless_number* out, double v);
LIMITLESS_API limitless_status limitless_number_from_cstr(limitless_ctx* ctx, limitless_number* out, const char* s, int base);
LIMITLESS_API limitless_status limitless_number_from_str(limitless_ctx* ctx, limitless_number* out, const char* s);
LIMITLESS_API limitless_status limitless_number_to_cstr(limitless_ctx* ctx, const limitless_number* n, int base, char* buf, limitless_size cap, limitless_size* written);
LIMITLESS_API limitless_status limitless_number_to_str(limitless_ctx* ctx, const limitless_number* n, char* buf, limitless_size cap, limitless_size* written);
LIMITLESS_API limitless_status limitless_number_to_i64(limitless_ctx* ctx, const limitless_number* n, limitless_i64* out);
LIMITLESS_API limitless_status limitless_number_to_u64(limitless_ctx* ctx, const limitless_number* n, limitless_u64* out);

LIMITLESS_API limitless_status limitless_number_add(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b);
LIMITLESS_API limitless_status limitless_number_sub(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b);
LIMITLESS_API limitless_status limitless_number_mul(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b);
LIMITLESS_API limitless_status limitless_number_div(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b);
LIMITLESS_API limitless_status limitless_number_neg(limitless_ctx* ctx, limitless_number* out, const limitless_number* a);
LIMITLESS_API limitless_status limitless_number_abs(limitless_ctx* ctx, limitless_number* out, const limitless_number* a);
LIMITLESS_API int limitless_number_cmp(limitless_ctx* ctx, const limitless_number* a, const limitless_number* b, limitless_status* st);

LIMITLESS_API limitless_status limitless_number_gcd(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b);
LIMITLESS_API limitless_status limitless_number_pow_u64(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, limitless_u64 exp);
LIMITLESS_API limitless_status limitless_number_modexp_u64(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, limitless_u64 exp, const limitless_number* mod);

LIMITLESS_API int limitless_number_is_zero(const limitless_number* n);
LIMITLESS_API int limitless_number_is_integer(const limitless_number* n);
LIMITLESS_API int limitless_number_sign(const limitless_number* n);

#ifdef __cplusplus
}
#endif

#ifdef LIMITLESS_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

#define LIMITLESS__DEFAULT_KARATSUBA_THRESHOLD ((limitless_size)32)

#ifndef LIMITLESS_DEFAULT_ALLOC
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4565)
#endif
extern void* malloc(limitless_size size);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#define LIMITLESS_DEFAULT_ALLOC(size) malloc((size))
#endif

#ifndef LIMITLESS_DEFAULT_REALLOC
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4565)
#endif
extern void* realloc(void* ptr, limitless_size size);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#define LIMITLESS_DEFAULT_REALLOC(ptr, old_size, new_size) realloc((ptr), (new_size))
#endif

#ifndef LIMITLESS_DEFAULT_FREE
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4565)
#endif
extern void free(void* ptr);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#define LIMITLESS_DEFAULT_FREE(ptr, size) free((ptr))
#endif

/* GCOVR_EXCL_START */
static void limitless__mem_zero(void* p, limitless_size n) {
  limitless_size i;
  limitless_u8* b = (limitless_u8*)p;
  for (i = 0; i < n; ++i) {
    b[i] = (limitless_u8)0;
  }
}

static void limitless__mem_copy(void* dst, const void* src, limitless_size n) {
  limitless_size i;
  limitless_u8* d = (limitless_u8*)dst;
  const limitless_u8* s = (const limitless_u8*)src;
  if (n == 0) return;
  if (!d || !s) return;
  for (i = 0; i < n; ++i) {
    d[i] = s[i];
  }
}

static int limitless__is_space(char c) {
  return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\f') || (c == '\v');
}

static int limitless__digit_val(char c) {
  if (c >= '0' && c <= '9') return (int)(c - '0');
  if (c >= 'a' && c <= 'z') return (int)(c - 'a' + 10);
  if (c >= 'A' && c <= 'Z') return (int)(c - 'A' + 10);
  return -1;
}

static char limitless__digit_chr(int v) {
  if (v < 10) return (char)('0' + v);
  return (char)('a' + (v - 10));
}

#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
static limitless_u32 limitless__ctz_u32(limitless_u32 x) {
  limitless_u32 n = 0;
  while ((x & 1u) == 0u) {
    x >>= 1;
    ++n;
  }
  return n;
}

static limitless_u32 limitless__clz_u32(limitless_u32 x) {
  limitless_u32 n = 0;
  if (x == 0u) return 32u;
  while ((x & 0x80000000u) == 0u) {
    x <<= 1;
    ++n;
  }
  return n;
}
#else
static limitless_u32 limitless__ctz_u64(limitless_u64 x) {
  limitless_u32 n = 0;
  while ((x & (limitless_u64)1u) == (limitless_u64)0u) {
    x >>= 1;
    ++n;
  }
  return n;
}

static limitless_u32 limitless__clz_u64(limitless_u64 x) {
  limitless_u32 n = 0;
  if (x == (limitless_u64)0u) return 64u;
  while ((x & (((limitless_u64)1u) << 63)) == (limitless_u64)0u) {
    x <<= 1;
    ++n;
  }
  return n;
}
#endif

static int limitless__alloc_valid(const limitless_alloc* a) {
  return a && a->alloc && a->realloc && a->free;
}

static void* limitless__alloc_bytes(limitless_ctx* ctx, limitless_size size) {
  if (!ctx || !ctx->allocator.alloc) return NULL;
  return ctx->allocator.alloc(ctx->allocator.user, size);
}

static void* limitless__realloc_bytes(limitless_ctx* ctx, void* ptr, limitless_size old_size, limitless_size new_size) {
  if (!ctx || !ctx->allocator.realloc) return NULL;
  return ctx->allocator.realloc(ctx->allocator.user, ptr, old_size, new_size);
}

static void limitless__free_bytes(limitless_ctx* ctx, void* ptr, limitless_size size) {
  if (!ctx || !ctx->allocator.free || !ptr) return;
  ctx->allocator.free(ctx->allocator.user, ptr, size);
}

static void* limitless__default_alloc_fn(void* user, limitless_size size) {
  (void)user;
  return LIMITLESS_DEFAULT_ALLOC(size);
}

static void* limitless__default_realloc_fn(void* user, void* ptr, limitless_size old_size, limitless_size new_size) {
  (void)user;
  (void)old_size;
  return LIMITLESS_DEFAULT_REALLOC(ptr, old_size, new_size);
}

static void limitless__default_free_fn(void* user, void* ptr, limitless_size size) {
  (void)user;
  (void)size;
  LIMITLESS_DEFAULT_FREE(ptr, size);
}

static void limitless__bigint_init_raw(limitless_bigint* a) {
  a->sign = 0;
  a->used = 0;
  a->cap = 0;
  a->limbs = NULL;
}

static void limitless__bigint_norm(limitless_bigint* a) {
  if (a->used > 0 && !a->limbs) {
    a->used = 0;
    a->sign = 0;
    return;
  }
  while (a->used > 0 && a->limbs[a->used - 1] == (limitless_limb)0) {
    --a->used;
  }
  if (a->used == 0) {
    a->sign = 0;
  }
}

static limitless_status limitless__bigint_reserve(limitless_ctx* ctx, limitless_bigint* a, limitless_size need) {
  limitless_size new_cap;
  limitless_size old_bytes;
  limitless_size new_bytes;
  void* mem;
  if (need == 0) return LIMITLESS_OK;
  if (!a->limbs) a->cap = 0;
  if (need <= a->cap && a->limbs) return LIMITLESS_OK;
  new_cap = (a->cap > 0) ? a->cap : (limitless_size)1;
  while (new_cap < need) {
    if (new_cap > ((~(limitless_size)0) / 2u)) {
      new_cap = need;
      break;
    }
    new_cap *= 2u;
  }
  old_bytes = a->cap * (limitless_size)sizeof(limitless_limb);
  new_bytes = new_cap * (limitless_size)sizeof(limitless_limb);
  if (a->limbs) {
    mem = limitless__realloc_bytes(ctx, a->limbs, old_bytes, new_bytes);
  } else {
    mem = limitless__alloc_bytes(ctx, new_bytes);
  }
  if (!mem) return LIMITLESS_EOOM;
  a->limbs = (limitless_limb*)mem;
  if (new_cap > a->cap) {
    limitless__mem_zero(a->limbs + a->cap, (new_cap - a->cap) * (limitless_size)sizeof(limitless_limb));
  }
  a->cap = new_cap;
  return LIMITLESS_OK;
}

static void limitless__bigint_clear_raw(limitless_ctx* ctx, limitless_bigint* a) {
  if (a->limbs) {
    limitless__free_bytes(ctx, a->limbs, a->cap * (limitless_size)sizeof(limitless_limb));
  }
  limitless__bigint_init_raw(a);
}

static limitless_status limitless__bigint_copy(limitless_ctx* ctx, limitless_bigint* dst, const limitless_bigint* src) {
  limitless_status st;
  if (dst == src) return LIMITLESS_OK;
  if (src->used > 0 && !src->limbs) return LIMITLESS_EINVAL;
  if (src->used == 0) {
    dst->used = 0;
    dst->sign = 0;
    return LIMITLESS_OK;
  }
  st = limitless__bigint_reserve(ctx, dst, src->used);
  if (st != LIMITLESS_OK) return st;
  if (!dst->limbs) return LIMITLESS_EOOM;
  limitless__mem_copy(dst->limbs, src->limbs, src->used * (limitless_size)sizeof(limitless_limb));
  dst->used = src->used;
  dst->sign = src->sign;
  return LIMITLESS_OK;
}

static void limitless__bigint_swap(limitless_bigint* a, limitless_bigint* b) {
  limitless_bigint t = *a;
  *a = *b;
  *b = t;
}

static int limitless__bigint_is_zero(const limitless_bigint* a) {
  return a->used == 0;
}

static int limitless__bigint_is_one(const limitless_bigint* a) {
  return a->sign > 0 && a->used == 1 && a->limbs[0] == (limitless_limb)1;
}

static limitless_size limitless__mag_used(const limitless_bigint* a) {
  if (!a->limbs) return 0;
  limitless_size u = a->used;
  while (u > 0 && a->limbs[u - 1] == (limitless_limb)0) {
    --u;
  }
  return u;
}

static int limitless__mag_cmp(const limitless_bigint* a, const limitless_bigint* b) {
  limitless_size au = limitless__mag_used(a);
  limitless_size bu = limitless__mag_used(b);
  limitless_size i;
  if (au < bu) return -1;
  if (au > bu) return 1;
  i = au;
  while (i > 0) {
    limitless_limb la = a->limbs[i - 1];
    limitless_limb lb = b->limbs[i - 1];
    if (la < lb) return -1;
    if (la > lb) return 1;
    --i;
  }
  return 0;
}

static int limitless__bigint_cmp_signed(const limitless_bigint* a, const limitless_bigint* b) {
  int as;
  int bs;
  int m;
  limitless_size au = limitless__mag_used(a);
  limitless_size bu = limitless__mag_used(b);
  as = (au == 0) ? 0 : a->sign;
  bs = (bu == 0) ? 0 : b->sign;
  if (as == 0 && au != 0) as = 1;
  if (bs == 0 && bu != 0) bs = 1;
  if (as < bs) return -1;
  if (as > bs) return 1;
  if (as == 0) return 0;
  m = limitless__mag_cmp(a, b);
  return (as > 0) ? m : -m;
}

static limitless_status limitless__bigint_set_u64(limitless_ctx* ctx, limitless_bigint* a, limitless_u64 v) {
  limitless_status st;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
  limitless_size need = (v > 0xffffffffULL) ? (limitless_size)2 : (v ? (limitless_size)1 : (limitless_size)0);
#else
  limitless_size need = (v ? (limitless_size)1 : (limitless_size)0);
#endif
  st = limitless__bigint_reserve(ctx, a, need);
  if (st != LIMITLESS_OK) return st;
  if (need == 0) {
    a->used = 0;
    a->sign = 0;
    return LIMITLESS_OK;
  }
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
  a->limbs[0] = (limitless_limb)(v & 0xffffffffULL);
  if (need > 1) a->limbs[1] = (limitless_limb)(v >> 32);
#else
  a->limbs[0] = (limitless_limb)v;
#endif
  a->used = need;
  a->sign = 1;
  limitless__bigint_norm(a);
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_set_i64(limitless_ctx* ctx, limitless_bigint* a, limitless_i64 v) {
  limitless_u64 mag;
  limitless_status st;
  if (v >= 0) {
    return limitless__bigint_set_u64(ctx, a, (limitless_u64)v);
  }
  /* two's-complement safe magnitude extraction */
  mag = ((limitless_u64)(~(limitless_u64)v)) + 1ULL;
  st = limitless__bigint_set_u64(ctx, a, mag);
  if (st != LIMITLESS_OK) return st;
  if (!limitless__bigint_is_zero(a)) a->sign = -1;
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_abs_copy(limitless_ctx* ctx, limitless_bigint* dst, const limitless_bigint* src) {
  limitless_status st = limitless__bigint_copy(ctx, dst, src);
  if (st != LIMITLESS_OK) return st;
  if (dst->sign < 0) dst->sign = 1;
  return LIMITLESS_OK;
}

static limitless_status limitless__mag_add(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_size i;
  limitless_size max_used = (a->used > b->used) ? a->used : b->used;
  limitless_status st;
  if (max_used == ~(limitless_size)0) return LIMITLESS_EOOM;
  st = limitless__bigint_reserve(ctx, out, max_used + 1);
  limitless_dlimb carry = (limitless_dlimb)0;
  if (st != LIMITLESS_OK) return st;
  for (i = 0; i < max_used; ++i) {
    limitless_dlimb av = (i < a->used) ? (limitless_dlimb)a->limbs[i] : (limitless_dlimb)0;
    limitless_dlimb bv = (i < b->used) ? (limitless_dlimb)b->limbs[i] : (limitless_dlimb)0;
    limitless_dlimb sum = av + bv + carry;
    out->limbs[i] = (limitless_limb)sum;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
    carry = sum >> 32;
#else
    carry = sum >> 64;
#endif
  }
  if (carry != (limitless_dlimb)0) {
    out->limbs[max_used] = (limitless_limb)carry;
    out->used = max_used + 1;
  } else {
    out->used = max_used;
  }
  out->sign = (out->used == 0) ? 0 : 1;
  limitless__bigint_norm(out);
  return LIMITLESS_OK;
}

static void limitless__mag_sub_inplace(limitless_bigint* a, const limitless_bigint* b) {
  limitless_size i;
  limitless_dlimb borrow = (limitless_dlimb)0;
  for (i = 0; i < a->used; ++i) {
    limitless_dlimb av = (limitless_dlimb)a->limbs[i];
    limitless_dlimb bv = (i < b->used) ? (limitless_dlimb)b->limbs[i] : (limitless_dlimb)0;
    limitless_dlimb sub = bv + borrow;
    if (av < sub) {
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
      a->limbs[i] = (limitless_limb)((((limitless_dlimb)1ULL << 32) + av) - sub);
#else
      a->limbs[i] = (limitless_limb)((((limitless_dlimb)1) << 64) + av - sub);
#endif
      borrow = (limitless_dlimb)1;
    } else {
      a->limbs[i] = (limitless_limb)(av - sub);
      borrow = (limitless_dlimb)0;
    }
  }
  limitless__bigint_norm(a);
  if (a->used > 0) a->sign = 1;
}

static limitless_status limitless__mag_sub(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_status st;
  limitless_size i;
  limitless_dlimb borrow = (limitless_dlimb)0;
  /* requires |a| >= |b| */
  st = limitless__bigint_reserve(ctx, out, a->used);
  if (st != LIMITLESS_OK) return st;
  for (i = 0; i < a->used; ++i) {
    limitless_dlimb av = (limitless_dlimb)a->limbs[i];
    limitless_dlimb bv = (i < b->used) ? (limitless_dlimb)b->limbs[i] : (limitless_dlimb)0;
    limitless_dlimb sub = bv + borrow;
    if (av < sub) {
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
      out->limbs[i] = (limitless_limb)((((limitless_dlimb)1ULL << 32) + av) - sub);
#else
      out->limbs[i] = (limitless_limb)((((limitless_dlimb)1) << 64) + av - sub);
#endif
      borrow = (limitless_dlimb)1;
    } else {
      out->limbs[i] = (limitless_limb)(av - sub);
      borrow = (limitless_dlimb)0;
    }
  }
  out->used = a->used;
  out->sign = (out->used == 0) ? 0 : 1;
  limitless__bigint_norm(out);
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_shl_bits(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, limitless_size bits) {
  limitless_size limb_shift;
  limitless_size bit_shift;
  limitless_size i;
  limitless_status st;
  limitless_dlimb carry = (limitless_dlimb)0;
  if (out == a) {
    limitless_bigint tmp;
    limitless__bigint_init_raw(&tmp);
    st = limitless__bigint_shl_bits(ctx, &tmp, a, bits);
    if (st == LIMITLESS_OK) {
      limitless__bigint_swap(out, &tmp);
    }
    limitless__bigint_clear_raw(ctx, &tmp);
    return st;
  }
  if (a->used == 0) {
    out->used = 0;
    out->sign = 0;
    return LIMITLESS_OK;
  }
  limb_shift = bits / (limitless_size)LIMITLESS_LIMB_BITS;
  bit_shift = bits % (limitless_size)LIMITLESS_LIMB_BITS;
  st = limitless__bigint_reserve(ctx, out, a->used + limb_shift + 1);
  if (st != LIMITLESS_OK) return st;
  for (i = 0; i < limb_shift; ++i) out->limbs[i] = (limitless_limb)0;
  for (i = 0; i < a->used; ++i) {
    limitless_dlimb cur = ((limitless_dlimb)a->limbs[i] << bit_shift) | carry;
    out->limbs[i + limb_shift] = (limitless_limb)cur;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
    carry = (bit_shift == 0) ? (limitless_dlimb)0 : (cur >> 32);
#else
    carry = (bit_shift == 0) ? (limitless_dlimb)0 : (cur >> 64);
#endif
  }
  out->used = a->used + limb_shift;
  if (carry != (limitless_dlimb)0) {
    out->limbs[out->used] = (limitless_limb)carry;
    ++out->used;
  }
  out->sign = a->sign;
  limitless__bigint_norm(out);
  return LIMITLESS_OK;
}

static void limitless__bigint_shr1_inplace(limitless_bigint* a) {
  limitless_size i = a->used;
  limitless_limb carry = (limitless_limb)0;
  while (i > 0) {
    limitless_limb cur = a->limbs[i - 1];
    a->limbs[i - 1] = (limitless_limb)((cur >> 1) | (carry << (LIMITLESS_LIMB_BITS - 1)));
    carry = (limitless_limb)(cur & (limitless_limb)1);
    --i;
  }
  limitless__bigint_norm(a);
}

static void limitless__bigint_shr_bits_inplace(limitless_bigint* a, limitless_size bits) {
  limitless_size limb_shift;
  limitless_size bit_shift;
  limitless_size i;
  if (a->used == 0 || bits == 0) return;
  limb_shift = bits / (limitless_size)LIMITLESS_LIMB_BITS;
  bit_shift = bits % (limitless_size)LIMITLESS_LIMB_BITS;
  if (limb_shift >= a->used) {
    a->used = 0;
    a->sign = 0;
    return;
  }
  if (limb_shift > 0) {
    for (i = 0; i + limb_shift < a->used; ++i) {
      a->limbs[i] = a->limbs[i + limb_shift];
    }
    a->used -= limb_shift;
  }
  if (bit_shift > 0) {
    limitless_limb carry = (limitless_limb)0;
    i = a->used;
    while (i > 0) {
      limitless_limb cur = a->limbs[i - 1];
      a->limbs[i - 1] = (limitless_limb)((cur >> bit_shift) | (carry << (LIMITLESS_LIMB_BITS - bit_shift)));
      carry = cur & (limitless_limb)((((limitless_limb)1) << bit_shift) - 1);
      --i;
    }
  }
  limitless__bigint_norm(a);
}

static limitless_size limitless__bigint_bit_length(const limitless_bigint* a) {
  limitless_limb top;
  limitless_u32 lz;
  if (a->used == 0) return 0;
  top = a->limbs[a->used - 1];
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
  lz = limitless__clz_u32((limitless_u32)top);
#else
  lz = limitless__clz_u64((limitless_u64)top);
#endif
  return (a->used - 1) * (limitless_size)LIMITLESS_LIMB_BITS + ((limitless_size)LIMITLESS_LIMB_BITS - (limitless_size)lz);
}

static limitless_size limitless__bigint_ctz_bits(const limitless_bigint* a) {
  limitless_size i = 0;
  limitless_size z = 0;
  while (i < a->used && a->limbs[i] == (limitless_limb)0) {
    z += (limitless_size)LIMITLESS_LIMB_BITS;
    ++i;
  }
  if (i == a->used) return z;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
  z += (limitless_size)limitless__ctz_u32((limitless_u32)a->limbs[i]);
#else
  z += (limitless_size)limitless__ctz_u64((limitless_u64)a->limbs[i]);
#endif
  return z;
}

static limitless_status limitless__bigint_set_bit(limitless_ctx* ctx, limitless_bigint* a, limitless_size bit) {
  limitless_size limb = bit / (limitless_size)LIMITLESS_LIMB_BITS;
  limitless_size off = bit % (limitless_size)LIMITLESS_LIMB_BITS;
  limitless_status st = limitless__bigint_reserve(ctx, a, limb + 1);
  if (st != LIMITLESS_OK) return st;
  while (a->used <= limb) {
    a->limbs[a->used++] = (limitless_limb)0;
  }
  a->limbs[limb] |= (limitless_limb)(((limitless_limb)1) << off);
  a->sign = (a->used == 0) ? 0 : 1;
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_mul_small_inplace(limitless_ctx* ctx, limitless_bigint* a, limitless_u32 m) {
  limitless_size i;
  limitless_dlimb carry = (limitless_dlimb)0;
  limitless_status st;
  if (a->used == 0 || m == 1u) return LIMITLESS_OK;
  if (m == 0u) {
    a->used = 0;
    a->sign = 0;
    return LIMITLESS_OK;
  }
  st = limitless__bigint_reserve(ctx, a, a->used + 1);
  if (st != LIMITLESS_OK) return st;
  for (i = 0; i < a->used; ++i) {
    limitless_dlimb prod = ((limitless_dlimb)a->limbs[i] * (limitless_dlimb)m) + carry;
    a->limbs[i] = (limitless_limb)prod;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
    carry = prod >> 32;
#else
    carry = prod >> 64;
#endif
  }
  if (carry != (limitless_dlimb)0) {
    a->limbs[a->used++] = (limitless_limb)carry;
  }
  if (a->used > 0 && a->sign == 0) a->sign = 1;
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_add_small_inplace(limitless_ctx* ctx, limitless_bigint* a, limitless_u32 v) {
  limitless_size i;
  limitless_dlimb carry;
  limitless_status st;
  if (v == 0u) return LIMITLESS_OK;
  if (a->used == 0) {
    return limitless__bigint_set_u64(ctx, a, (limitless_u64)v);
  }
  st = limitless__bigint_reserve(ctx, a, a->used + 1);
  if (st != LIMITLESS_OK) return st;
  carry = (limitless_dlimb)v;
  i = 0;
  while (carry != (limitless_dlimb)0 && i < a->used) {
    limitless_dlimb sum = (limitless_dlimb)a->limbs[i] + carry;
    a->limbs[i] = (limitless_limb)sum;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
    carry = sum >> 32;
#else
    carry = sum >> 64;
#endif
    ++i;
  }
  if (carry != (limitless_dlimb)0) {
    a->limbs[a->used++] = (limitless_limb)carry;
  }
  if (a->used > 0 && a->sign == 0) a->sign = 1;
  return LIMITLESS_OK;
}

static limitless_u32 limitless__bigint_divmod_small_inplace(limitless_bigint* a, limitless_u32 base) {
  limitless_size i = a->used;
  limitless_dlimb rem = (limitless_dlimb)0;
  while (i > 0) {
    limitless_dlimb cur;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
    cur = (rem << 32) | (limitless_dlimb)a->limbs[i - 1];
#else
    cur = (rem << 64) | (limitless_dlimb)a->limbs[i - 1];
#endif
    a->limbs[i - 1] = (limitless_limb)(cur / (limitless_dlimb)base);
    rem = cur % (limitless_dlimb)base;
    --i;
  }
  limitless__bigint_norm(a);
  return (limitless_u32)rem;
}

static limitless_status limitless__bigint_slice(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, limitless_size start, limitless_size count) {
  limitless_size i;
  limitless_size end = start + count;
  limitless_size actual;
  limitless_status st;
  if (start >= a->used) {
    out->used = 0;
    out->sign = 0;
    return LIMITLESS_OK;
  }
  if (end > a->used) end = a->used;
  actual = end - start;
  st = limitless__bigint_reserve(ctx, out, actual);
  if (st != LIMITLESS_OK) return st;
  for (i = 0; i < actual; ++i) out->limbs[i] = a->limbs[start + i];
  out->used = actual;
  out->sign = (actual == 0) ? 0 : 1;
  limitless__bigint_norm(out);
  return LIMITLESS_OK;
}

static limitless_status limitless__mag_mul_schoolbook(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_size i;
  limitless_size j;
  limitless_status st;
  limitless_size need;
  if (a->used == 0 || b->used == 0) {
    out->used = 0;
    out->sign = 0;
    return LIMITLESS_OK;
  }
  need = a->used + b->used;
  st = limitless__bigint_reserve(ctx, out, need);
  if (st != LIMITLESS_OK) return st;
  for (i = 0; i < need; ++i) out->limbs[i] = (limitless_limb)0;
  for (i = 0; i < a->used; ++i) {
    limitless_dlimb carry = (limitless_dlimb)0;
    for (j = 0; j < b->used; ++j) {
      limitless_dlimb cur = (limitless_dlimb)out->limbs[i + j] + (limitless_dlimb)a->limbs[i] * (limitless_dlimb)b->limbs[j] + carry;
      out->limbs[i + j] = (limitless_limb)cur;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
      carry = cur >> 32;
#else
      carry = cur >> 64;
#endif
    }
    out->limbs[i + b->used] = (limitless_limb)((limitless_dlimb)out->limbs[i + b->used] + carry);
  }
  out->used = need;
  out->sign = 1;
  limitless__bigint_norm(out);
  return LIMITLESS_OK;
}

static limitless_status limitless__mag_add_shifted(limitless_ctx* ctx, limitless_bigint* acc, const limitless_bigint* term, limitless_size limb_shift) {
  limitless_size i;
  limitless_size need;
  limitless_dlimb carry = (limitless_dlimb)0;
  limitless_status st;
  if (term->used == 0) return LIMITLESS_OK;
  need = (acc->used > term->used + limb_shift) ? acc->used : (term->used + limb_shift);
  st = limitless__bigint_reserve(ctx, acc, need + 1);
  if (st != LIMITLESS_OK) return st;
  if (acc->used < need) {
    for (i = acc->used; i < need; ++i) acc->limbs[i] = (limitless_limb)0;
    acc->used = need;
  }
  for (i = 0; i < term->used; ++i) {
    limitless_size idx = i + limb_shift;
    limitless_dlimb sum = (limitless_dlimb)acc->limbs[idx] + (limitless_dlimb)term->limbs[i] + carry;
    acc->limbs[idx] = (limitless_limb)sum;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
    carry = sum >> 32;
#else
    carry = sum >> 64;
#endif
    if (carry == (limitless_dlimb)0) {
      while (++i < term->used) {
        idx = i + limb_shift;
        sum = (limitless_dlimb)acc->limbs[idx] + (limitless_dlimb)term->limbs[i];
        acc->limbs[idx] = (limitless_limb)sum;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
        carry = sum >> 32;
#else
        carry = sum >> 64;
#endif
        if (carry != (limitless_dlimb)0) break;
      }
      break;
    }
  }
  if (carry != (limitless_dlimb)0) {
    limitless_size idx = term->used + limb_shift;
    while (carry != (limitless_dlimb)0) {
      limitless_dlimb sum = (limitless_dlimb)acc->limbs[idx] + carry;
      acc->limbs[idx] = (limitless_limb)sum;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
      carry = sum >> 32;
#else
      carry = sum >> 64;
#endif
      ++idx;
      if (idx > acc->used) {
        acc->used = idx;
      }
      if (idx == acc->cap) {
        st = limitless__bigint_reserve(ctx, acc, acc->cap + 1);
        if (st != LIMITLESS_OK) return st;
      }
    }
  }
  limitless__bigint_norm(acc);
  if (acc->used > 0) acc->sign = 1;
  return LIMITLESS_OK;
}

static limitless_status limitless__mag_mul_karatsuba(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b, limitless_size threshold);

static limitless_status limitless__mag_mul(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b, limitless_size threshold) {
  limitless_size maxu = (a->used > b->used) ? a->used : b->used;
  limitless_size minu = (a->used < b->used) ? a->used : b->used;
  if (maxu < threshold || minu < 2) {
    return limitless__mag_mul_schoolbook(ctx, out, a, b);
  }
  return limitless__mag_mul_karatsuba(ctx, out, a, b, threshold);
}

static limitless_status limitless__mag_mul_karatsuba(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b, limitless_size threshold) {
  limitless_status st;
  limitless_size n = (a->used > b->used) ? a->used : b->used;
  limitless_size m;
  limitless_bigint a0, a1, b0, b1, z0, z1, z2, s1, s2, p, tmp;

  if (n < threshold || a->used < 2 || b->used < 2) {
    return limitless__mag_mul_schoolbook(ctx, out, a, b);
  }

  m = n / 2;
  limitless__bigint_init_raw(&a0); limitless__bigint_init_raw(&a1);
  limitless__bigint_init_raw(&b0); limitless__bigint_init_raw(&b1);
  limitless__bigint_init_raw(&z0); limitless__bigint_init_raw(&z1); limitless__bigint_init_raw(&z2);
  limitless__bigint_init_raw(&s1); limitless__bigint_init_raw(&s2); limitless__bigint_init_raw(&p);
  limitless__bigint_init_raw(&tmp);

  st = limitless__bigint_slice(ctx, &a0, a, 0, m); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__bigint_slice(ctx, &a1, a, m, n - m); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__bigint_slice(ctx, &b0, b, 0, m); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__bigint_slice(ctx, &b1, b, m, n - m); if (st != LIMITLESS_OK) goto cleanup;

  st = limitless__mag_mul(ctx, &z0, &a0, &b0, threshold); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__mag_mul(ctx, &z2, &a1, &b1, threshold); if (st != LIMITLESS_OK) goto cleanup;

  st = limitless__mag_add(ctx, &s1, &a0, &a1); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__mag_add(ctx, &s2, &b0, &b1); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__mag_mul(ctx, &p, &s1, &s2, threshold); if (st != LIMITLESS_OK) goto cleanup;

  st = limitless__mag_sub(ctx, &tmp, &p, &z0); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__mag_sub(ctx, &z1, &tmp, &z2); if (st != LIMITLESS_OK) goto cleanup;

  out->used = 0;
  out->sign = 0;
  st = limitless__mag_add_shifted(ctx, out, &z0, 0); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__mag_add_shifted(ctx, out, &z1, m); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__mag_add_shifted(ctx, out, &z2, m * 2); if (st != LIMITLESS_OK) goto cleanup;
  out->sign = (out->used == 0) ? 0 : 1;

cleanup:
  limitless__bigint_clear_raw(ctx, &a0); limitless__bigint_clear_raw(ctx, &a1);
  limitless__bigint_clear_raw(ctx, &b0); limitless__bigint_clear_raw(ctx, &b1);
  limitless__bigint_clear_raw(ctx, &z0); limitless__bigint_clear_raw(ctx, &z1); limitless__bigint_clear_raw(ctx, &z2);
  limitless__bigint_clear_raw(ctx, &s1); limitless__bigint_clear_raw(ctx, &s2); limitless__bigint_clear_raw(ctx, &p);
  limitless__bigint_clear_raw(ctx, &tmp);
  return st;
}

static limitless_status limitless__bigint_add_signed(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_status st;
  if (a->sign == 0) return limitless__bigint_copy(ctx, out, b);
  if (b->sign == 0) return limitless__bigint_copy(ctx, out, a);
  if (a->sign == b->sign) {
    st = limitless__mag_add(ctx, out, a, b);
    if (st != LIMITLESS_OK) return st;
    out->sign = a->sign;
    return LIMITLESS_OK;
  }
  {
    int cmp = limitless__mag_cmp(a, b);
    if (cmp == 0) {
      out->used = 0;
      out->sign = 0;
      return LIMITLESS_OK;
    }
    if (cmp > 0) {
      st = limitless__mag_sub(ctx, out, a, b);
      if (st != LIMITLESS_OK) return st;
      out->sign = a->sign;
    } else {
      st = limitless__mag_sub(ctx, out, b, a);
      if (st != LIMITLESS_OK) return st;
      out->sign = b->sign;
    }
  }
  limitless__bigint_norm(out);
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_sub_signed(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_bigint nb;
  limitless_status st;
  limitless__bigint_init_raw(&nb);
  st = limitless__bigint_copy(ctx, &nb, b);
  if (st != LIMITLESS_OK) {
    limitless__bigint_clear_raw(ctx, &nb);
    return st;
  }
  nb.sign = -nb.sign;
  st = limitless__bigint_add_signed(ctx, out, a, &nb);
  limitless__bigint_clear_raw(ctx, &nb);
  return st;
}

static limitless_status limitless__bigint_mul_signed(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_status st;
  if (a->used == 0 || b->used == 0) {
    out->used = 0;
    out->sign = 0;
    return LIMITLESS_OK;
  }
  st = limitless__mag_mul(ctx, out, a, b, ctx->karatsuba_threshold);
  if (st != LIMITLESS_OK) return st;
  out->sign = (a->sign == b->sign) ? 1 : -1;
  limitless__bigint_norm(out);
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_divmod_abs(limitless_ctx* ctx, limitless_bigint* q, limitless_bigint* r, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_status st;
  int cmp;
  limitless_size abit;
  limitless_size bbit;
  limitless_size shift;
  limitless_size i;
  limitless_bigint rem, den;

  if (b->used == 0) return LIMITLESS_EDIVZERO;

  limitless__bigint_init_raw(&rem);
  limitless__bigint_init_raw(&den);

  st = limitless__bigint_abs_copy(ctx, &rem, a);
  if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__bigint_abs_copy(ctx, &den, b);
  if (st != LIMITLESS_OK) goto cleanup;

  cmp = limitless__mag_cmp(&rem, &den);
  if (cmp < 0) {
    q->used = 0;
    q->sign = 0;
    st = limitless__bigint_copy(ctx, r, &rem);
    if (st == LIMITLESS_OK) r->sign = (r->used == 0) ? 0 : 1;
    goto cleanup;
  }
  if (cmp == 0) {
    st = limitless__bigint_set_u64(ctx, q, 1);
    if (st != LIMITLESS_OK) goto cleanup;
    r->used = 0;
    r->sign = 0;
    goto cleanup;
  }

  abit = limitless__bigint_bit_length(&rem);
  bbit = limitless__bigint_bit_length(&den);
  shift = abit - bbit;

  st = limitless__bigint_shl_bits(ctx, &den, &den, shift);
  if (st != LIMITLESS_OK) goto cleanup;

  st = limitless__bigint_reserve(ctx, q, (shift / (limitless_size)LIMITLESS_LIMB_BITS) + 1);
  if (st != LIMITLESS_OK) goto cleanup;
  for (i = 0; i < q->cap; ++i) q->limbs[i] = (limitless_limb)0;
  q->used = (shift / (limitless_size)LIMITLESS_LIMB_BITS) + 1;
  q->sign = 1;

  i = shift + 1;
  while (i > 0) {
    limitless_size bit = i - 1;
    if (limitless__mag_cmp(&rem, &den) >= 0) {
      limitless__mag_sub_inplace(&rem, &den);
      st = limitless__bigint_set_bit(ctx, q, bit);
      if (st != LIMITLESS_OK) goto cleanup;
    }
    limitless__bigint_shr1_inplace(&den);
    --i;
  }

  st = limitless__bigint_copy(ctx, r, &rem);
  if (st != LIMITLESS_OK) goto cleanup;
  q->sign = (q->used == 0) ? 0 : 1;
  r->sign = (r->used == 0) ? 0 : 1;

cleanup:
  limitless__bigint_clear_raw(ctx, &rem);
  limitless__bigint_clear_raw(ctx, &den);
  return st;
}

static limitless_status limitless__bigint_divmod_signed(limitless_ctx* ctx, limitless_bigint* q, limitless_bigint* r, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_status st = limitless__bigint_divmod_abs(ctx, q, r, a, b);
  if (st != LIMITLESS_OK) return st;
  if (q->used != 0) q->sign = (a->sign == b->sign) ? 1 : -1;
  if (r->used != 0) r->sign = a->sign;
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_mod(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* m) {
  limitless_bigint q, r;
  limitless_status st;
  limitless__bigint_init_raw(&q);
  limitless__bigint_init_raw(&r);
  st = limitless__bigint_divmod_signed(ctx, &q, &r, a, m);
  if (st == LIMITLESS_OK) {
    st = limitless__bigint_copy(ctx, out, &r);
  }
  limitless__bigint_clear_raw(ctx, &q);
  limitless__bigint_clear_raw(ctx, &r);
  return st;
}

static limitless_status limitless__bigint_div_exact(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* d) {
  limitless_bigint q, r;
  limitless_status st;
  limitless__bigint_init_raw(&q);
  limitless__bigint_init_raw(&r);
  st = limitless__bigint_divmod_signed(ctx, &q, &r, a, d);
  if (st != LIMITLESS_OK) goto cleanup;
  if (r.used != 0) {
    st = LIMITLESS_EINVAL;
    goto cleanup;
  }
  st = limitless__bigint_copy(ctx, out, &q);
cleanup:
  limitless__bigint_clear_raw(ctx, &q);
  limitless__bigint_clear_raw(ctx, &r);
  return st;
}

static limitless_status limitless__bigint_gcd(limitless_ctx* ctx, limitless_bigint* out, const limitless_bigint* a, const limitless_bigint* b) {
  limitless_bigint u, v;
  limitless_size shift;
  limitless_status st;
  limitless__bigint_init_raw(&u);
  limitless__bigint_init_raw(&v);
  st = limitless__bigint_abs_copy(ctx, &u, a); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__bigint_abs_copy(ctx, &v, b); if (st != LIMITLESS_OK) goto cleanup;

  if (u.used == 0) {
    st = limitless__bigint_copy(ctx, out, &v);
    if (st == LIMITLESS_OK && out->used > 0) out->sign = 1;
    goto cleanup;
  }
  if (v.used == 0) {
    st = limitless__bigint_copy(ctx, out, &u);
    if (st == LIMITLESS_OK && out->used > 0) out->sign = 1;
    goto cleanup;
  }

  {
    limitless_size uz = limitless__bigint_ctz_bits(&u);
    limitless_size vz = limitless__bigint_ctz_bits(&v);
    shift = (uz < vz) ? uz : vz;
    limitless__bigint_shr_bits_inplace(&u, uz);
    limitless__bigint_shr_bits_inplace(&v, vz);
  }

  while (u.used != 0 && v.used != 0) {
    int c = limitless__mag_cmp(&u, &v);
    if (c == 0) break;
    if (c > 0) {
      limitless__mag_sub_inplace(&u, &v);
      limitless__bigint_shr_bits_inplace(&u, limitless__bigint_ctz_bits(&u));
    } else {
      limitless__mag_sub_inplace(&v, &u);
      limitless__bigint_shr_bits_inplace(&v, limitless__bigint_ctz_bits(&v));
    }
  }

  if (u.used == 0) {
    st = limitless__bigint_copy(ctx, out, &v);
  } else {
    st = limitless__bigint_copy(ctx, out, &u);
  }
  if (st != LIMITLESS_OK) goto cleanup;
  if (shift > 0) {
    limitless_bigint tmp;
    limitless__bigint_init_raw(&tmp);
    st = limitless__bigint_shl_bits(ctx, &tmp, out, shift);
    if (st == LIMITLESS_OK) {
      limitless__bigint_swap(out, &tmp);
    }
    limitless__bigint_clear_raw(ctx, &tmp);
    if (st != LIMITLESS_OK) goto cleanup;
  }
  if (out->used > 0) out->sign = 1;

cleanup:
  limitless__bigint_clear_raw(ctx, &u);
  limitless__bigint_clear_raw(ctx, &v);
  return st;
}

static limitless_status limitless__rational_init(limitless_rational* r) {
  limitless__bigint_init_raw(&r->num);
  limitless__bigint_init_raw(&r->den);
  return LIMITLESS_OK;
}

static void limitless__rational_clear(limitless_ctx* ctx, limitless_rational* r) {
  limitless__bigint_clear_raw(ctx, &r->num);
  limitless__bigint_clear_raw(ctx, &r->den);
}

static limitless_status limitless__rational_copy(limitless_ctx* ctx, limitless_rational* dst, const limitless_rational* src) {
  limitless_status st = limitless__bigint_copy(ctx, &dst->num, &src->num);
  if (st != LIMITLESS_OK) return st;
  st = limitless__bigint_copy(ctx, &dst->den, &src->den);
  if (st != LIMITLESS_OK) return st;
  return LIMITLESS_OK;
}

static limitless_status limitless__rational_set_int(limitless_ctx* ctx, limitless_rational* r, const limitless_bigint* i) {
  limitless_status st;
  st = limitless__bigint_copy(ctx, &r->num, i);
  if (st != LIMITLESS_OK) return st;
  st = limitless__bigint_set_u64(ctx, &r->den, 1);
  if (st != LIMITLESS_OK) return st;
  r->den.sign = 1;
  return LIMITLESS_OK;
}

static limitless_status limitless__rational_normalize(limitless_ctx* ctx, limitless_rational* r) {
  limitless_status st;
  limitless_bigint g, q;
  if (r->den.used == 0) return LIMITLESS_EDIVZERO;
  if (r->num.used == 0) {
    st = limitless__bigint_set_u64(ctx, &r->den, 1);
    if (st != LIMITLESS_OK) return st;
    r->num.sign = 0;
    r->den.sign = 1;
    return LIMITLESS_OK;
  }
  if (r->den.sign < 0) {
    r->den.sign = 1;
    r->num.sign = -r->num.sign;
  }

  limitless__bigint_init_raw(&g);
  limitless__bigint_init_raw(&q);

  st = limitless__bigint_gcd(ctx, &g, &r->num, &r->den);
  if (st != LIMITLESS_OK) goto cleanup;
  if (g.used != 0 && !limitless__bigint_is_one(&g)) {
    st = limitless__bigint_div_exact(ctx, &q, &r->num, &g);
    if (st != LIMITLESS_OK) goto cleanup;
    limitless__bigint_swap(&r->num, &q);
    q.used = 0;
    q.sign = 0;

    st = limitless__bigint_div_exact(ctx, &q, &r->den, &g);
    if (st != LIMITLESS_OK) goto cleanup;
    limitless__bigint_swap(&r->den, &q);
    q.used = 0;
    q.sign = 0;
  }

cleanup:
  limitless__bigint_clear_raw(ctx, &g);
  limitless__bigint_clear_raw(ctx, &q);
  return st;
}

static int limitless__rational_den_is_one(const limitless_rational* r) {
  return r->den.used == 1 && r->den.sign == 1 && r->den.limbs[0] == (limitless_limb)1;
}

static void limitless__number_swap(limitless_number* a, limitless_number* b) {
  limitless_number t = *a;
  *a = *b;
  *b = t;
}

static limitless_status limitless__number_to_rational_copy(limitless_ctx* ctx, limitless_rational* out, const limitless_number* n) {
  if (n->kind == LIMITLESS_KIND_INT) {
    return limitless__rational_set_int(ctx, out, &n->v.i);
  }
  if (n->kind == LIMITLESS_KIND_RAT) {
    return limitless__rational_copy(ctx, out, &n->v.r);
  }
  return LIMITLESS_EINVAL;
}

static limitless_status limitless__bigint_from_base_digits(limitless_ctx* ctx, limitless_bigint* out, const char* s, int base, const char** endp) {
  const char* p = s;
  int sign = 1;
  int actual_base = base;
  int any = 0;
  limitless_bigint v;
  limitless_status st;
  limitless__bigint_init_raw(&v);

  if (!s || base < 0 || base == 1 || base > 36) return LIMITLESS_EPARSE;

  while (*p && limitless__is_space(*p)) ++p;

  if (*p == '+' || *p == '-') {
    if (*p == '-') sign = -1;
    ++p;
  }

  if (actual_base == 0) {
    actual_base = 10;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
      actual_base = 16;
      p += 2;
    } else if (p[0] == '0' && (p[1] == 'b' || p[1] == 'B')) {
      actual_base = 2;
      p += 2;
    } else if (p[0] == '0' && p[1] != '\0') {
      actual_base = 8;
      p += 1;
    }
  } else {
    if (actual_base == 16 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) p += 2;
    if (actual_base == 2 && p[0] == '0' && (p[1] == 'b' || p[1] == 'B')) p += 2;
  }

  st = limitless__bigint_set_u64(ctx, &v, 0);
  if (st != LIMITLESS_OK) {
    limitless__bigint_clear_raw(ctx, &v);
    return st;
  }

  while (*p) {
    int d = limitless__digit_val(*p);
    if (d < 0 || d >= actual_base) break;
    any = 1;
    st = limitless__bigint_mul_small_inplace(ctx, &v, (limitless_u32)actual_base);
    if (st != LIMITLESS_OK) {
      limitless__bigint_clear_raw(ctx, &v);
      return st;
    }
    st = limitless__bigint_add_small_inplace(ctx, &v, (limitless_u32)d);
    if (st != LIMITLESS_OK) {
      limitless__bigint_clear_raw(ctx, &v);
      return st;
    }
    ++p;
  }

  if (!any) {
    limitless__bigint_clear_raw(ctx, &v);
    return LIMITLESS_EPARSE;
  }

  while (*p && limitless__is_space(*p)) ++p;

  if (sign < 0 && v.used != 0) v.sign = -1;

  st = limitless__bigint_copy(ctx, out, &v);
  limitless__bigint_clear_raw(ctx, &v);
  if (st != LIMITLESS_OK) return st;

  if (endp) *endp = p;
  return LIMITLESS_OK;
}

static limitless_status limitless__bigint_to_base_string(limitless_ctx* ctx, const limitless_bigint* a, int base, char** out_s, limitless_size* out_len) {
  limitless_bigint t;
  limitless_size bits;
  limitless_size cap = 0;
  limitless_size n = 0;
  char* rev = NULL;
  char* s = NULL;
  limitless_status st;

  if (base < 2 || base > 36 || !out_s || !out_len) return LIMITLESS_EINVAL;

  limitless__bigint_init_raw(&t);
  st = limitless__bigint_abs_copy(ctx, &t, a);
  if (st != LIMITLESS_OK) goto cleanup;

  bits = limitless__bigint_bit_length(&t);
  cap = (bits == 0) ? (limitless_size)1 : (bits + 1);

  rev = (char*)limitless__alloc_bytes(ctx, cap);
  if (!rev) {
    st = LIMITLESS_EOOM;
    goto cleanup;
  }

  if (t.used == 0) {
    rev[n++] = '0';
  } else {
    while (t.used != 0) {
      limitless_u32 rem = limitless__bigint_divmod_small_inplace(&t, (limitless_u32)base);
      if (n >= cap) {
        char* grown = (char*)limitless__realloc_bytes(ctx, rev, cap, cap * 2);
        if (!grown) {
          st = LIMITLESS_EOOM;
          goto cleanup;
        }
        rev = grown;
        cap *= 2;
      }
      rev[n++] = limitless__digit_chr((int)rem);
    }
  }

  s = (char*)limitless__alloc_bytes(ctx, n + 1);
  if (!s) {
    st = LIMITLESS_EOOM;
    goto cleanup;
  }

  {
    limitless_size i;
    for (i = 0; i < n; ++i) s[i] = rev[n - 1 - i];
    s[n] = '\0';
  }

  *out_s = s;
  *out_len = n;
  s = NULL;
  st = LIMITLESS_OK;

cleanup:
  if (rev) limitless__free_bytes(ctx, rev, cap);
  if (s) limitless__free_bytes(ctx, s, n + 1);
  limitless__bigint_clear_raw(ctx, &t);
  return st;
}

static limitless_status limitless__bigint_abs_to_u64(const limitless_bigint* a, limitless_u64* out) {
  if (!out) return LIMITLESS_EINVAL;
#if (LIMITLESS_LIMB_BITS == LIMITLESS_LIMB_BITS_32)
  if (a->used > 2) return LIMITLESS_ERANGE;
  if (a->used == 0) {
    *out = 0;
  } else if (a->used == 1) {
    *out = (limitless_u64)a->limbs[0];
  } else {
    *out = ((limitless_u64)a->limbs[1] << 32) | (limitless_u64)a->limbs[0];
  }
#else
  if (a->used > 1) return LIMITLESS_ERANGE;
  *out = (a->used == 0) ? (limitless_u64)0 : (limitless_u64)a->limbs[0];
#endif
  return LIMITLESS_OK;
}

static limitless_status limitless__number_get_integer_ref(const limitless_number* n, const limitless_bigint** i) {
  if (!n || !i) return LIMITLESS_EINVAL;
  if (n->kind == LIMITLESS_KIND_INT) {
    *i = &n->v.i;
    return LIMITLESS_OK;
  }
  if (n->kind == LIMITLESS_KIND_RAT && limitless__rational_den_is_one(&n->v.r)) {
    *i = &n->v.r.num;
    return LIMITLESS_OK;
  }
  return LIMITLESS_ETYPE;
}
/* GCOVR_EXCL_STOP */

LIMITLESS_API limitless_status limitless_ctx_init(limitless_ctx* ctx, const limitless_alloc* alloc) {
  if (!ctx || !limitless__alloc_valid(alloc)) return LIMITLESS_EINVAL;
  ctx->allocator = *alloc;
  ctx->karatsuba_threshold = LIMITLESS__DEFAULT_KARATSUBA_THRESHOLD;
  return LIMITLESS_OK;
}

LIMITLESS_API limitless_status limitless_ctx_init_default(limitless_ctx* ctx) {
  limitless_alloc a;
  if (!ctx) return LIMITLESS_EINVAL;
  a.alloc = limitless__default_alloc_fn;
  a.realloc = limitless__default_realloc_fn;
  a.free = limitless__default_free_fn;
  a.user = NULL;
  return limitless_ctx_init(ctx, &a);
}

LIMITLESS_API void limitless_ctx_set_karatsuba_threshold(limitless_ctx* ctx, limitless_size limbs) {
  if (!ctx) return;
  if (limbs < 2) limbs = 2;
  ctx->karatsuba_threshold = limbs;
}

LIMITLESS_API limitless_status limitless_number_init(limitless_ctx* ctx, limitless_number* n) {
  (void)ctx;
  if (!n) return LIMITLESS_EINVAL;
  n->kind = LIMITLESS_KIND_INT;
  limitless__bigint_init_raw(&n->v.i);
  return LIMITLESS_OK;
}

LIMITLESS_API void limitless_number_clear(limitless_ctx* ctx, limitless_number* n) {
  if (!n) return;
  if (n->kind == LIMITLESS_KIND_INT) {
    limitless__bigint_clear_raw(ctx, &n->v.i);
  } else if (n->kind == LIMITLESS_KIND_RAT) {
    limitless__rational_clear(ctx, &n->v.r);
  }
  n->kind = LIMITLESS_KIND_INT;
  limitless__bigint_init_raw(&n->v.i);
}

LIMITLESS_API limitless_status limitless_number_copy(limitless_ctx* ctx, limitless_number* dst, const limitless_number* src) {
  limitless_number tmp;
  limitless_status st;
  if (!ctx || !dst || !src) return LIMITLESS_EINVAL;
  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;

  if (src->kind == LIMITLESS_KIND_INT) {
    tmp.kind = LIMITLESS_KIND_INT;
    st = limitless__bigint_copy(ctx, &tmp.v.i, &src->v.i);
  } else if (src->kind == LIMITLESS_KIND_RAT) {
    tmp.kind = LIMITLESS_KIND_RAT;
    limitless__rational_init(&tmp.v.r);
    st = limitless__rational_copy(ctx, &tmp.v.r, &src->v.r);
  } else {
    st = LIMITLESS_EINVAL;
  }
  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    return st;
  }
  limitless__number_swap(dst, &tmp);
  limitless_number_clear(ctx, &tmp);
  return LIMITLESS_OK;
}

LIMITLESS_API limitless_status limitless_number_from_i64(limitless_ctx* ctx, limitless_number* out, limitless_i64 v) {
  limitless_number tmp;
  limitless_status st;
  if (!ctx || !out) return LIMITLESS_EINVAL;
  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;
  tmp.kind = LIMITLESS_KIND_INT;
  st = limitless__bigint_set_i64(ctx, &tmp.v.i, v);
  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    return st;
  }
  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);
  return LIMITLESS_OK;
}

LIMITLESS_API limitless_status limitless_number_from_u64(limitless_ctx* ctx, limitless_number* out, limitless_u64 v) {
  limitless_number tmp;
  limitless_status st;
  if (!ctx || !out) return LIMITLESS_EINVAL;
  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;
  tmp.kind = LIMITLESS_KIND_INT;
  st = limitless__bigint_set_u64(ctx, &tmp.v.i, v);
  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    return st;
  }
  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);
  return LIMITLESS_OK;
}

LIMITLESS_API limitless_status limitless_number_from_long(limitless_ctx* ctx, limitless_number* out, long v) {
  return limitless_number_from_i64(ctx, out, (limitless_i64)v);
}

LIMITLESS_API limitless_status limitless_number_from_ulong(limitless_ctx* ctx, limitless_number* out, unsigned long v) {
  return limitless_number_from_u64(ctx, out, (limitless_u64)v);
}

LIMITLESS_API limitless_status limitless_number_from_ll(limitless_ctx* ctx, limitless_number* out, long long v) {
  return limitless_number_from_i64(ctx, out, (limitless_i64)v);
}

LIMITLESS_API limitless_status limitless_number_from_ull(limitless_ctx* ctx, limitless_number* out, unsigned long long v) {
  return limitless_number_from_u64(ctx, out, (limitless_u64)v);
}

/* GCOVR_EXCL_START */
static limitless_status limitless__number_from_mantissa_exp2(limitless_ctx* ctx, limitless_number* out, int sign, limitless_u64 mantissa, int exp2) {
  limitless_number tmp;
  limitless_status st;
  if (!ctx || !out) return LIMITLESS_EINVAL;
  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;

  if (mantissa == 0ULL) {
    st = limitless_number_from_u64(ctx, &tmp, 0);
    if (st != LIMITLESS_OK) {
      limitless_number_clear(ctx, &tmp);
      return st;
    }
    limitless__number_swap(out, &tmp);
    limitless_number_clear(ctx, &tmp);
    return LIMITLESS_OK;
  }

  if (exp2 >= 0) {
    tmp.kind = LIMITLESS_KIND_INT;
    st = limitless__bigint_set_u64(ctx, &tmp.v.i, mantissa);
    if (st != LIMITLESS_OK) goto cleanup;
    {
      limitless_bigint shifted;
      limitless__bigint_init_raw(&shifted);
      st = limitless__bigint_shl_bits(ctx, &shifted, &tmp.v.i, (limitless_size)exp2);
      if (st == LIMITLESS_OK) {
        limitless__bigint_swap(&tmp.v.i, &shifted);
      }
      limitless__bigint_clear_raw(ctx, &shifted);
      if (st != LIMITLESS_OK) goto cleanup;
    }
    if (sign < 0 && tmp.v.i.used != 0) tmp.v.i.sign = -1;
  } else {
    limitless_rational r;
    limitless__rational_init(&r);
    st = limitless__bigint_set_u64(ctx, &r.num, mantissa);
    if (st != LIMITLESS_OK) {
      limitless__rational_clear(ctx, &r);
      goto cleanup;
    }
    st = limitless__bigint_set_u64(ctx, &r.den, 1);
    if (st != LIMITLESS_OK) {
      limitless__rational_clear(ctx, &r);
      goto cleanup;
    }
    {
      limitless_bigint d2;
      limitless__bigint_init_raw(&d2);
      st = limitless__bigint_shl_bits(ctx, &d2, &r.den, (limitless_size)(-exp2));
      if (st == LIMITLESS_OK) {
        limitless__bigint_swap(&r.den, &d2);
      }
      limitless__bigint_clear_raw(ctx, &d2);
      if (st != LIMITLESS_OK) {
        limitless__rational_clear(ctx, &r);
        goto cleanup;
      }
    }
    if (sign < 0 && r.num.used != 0) r.num.sign = -1;
    st = limitless__rational_normalize(ctx, &r);
    if (st != LIMITLESS_OK) {
      limitless__rational_clear(ctx, &r);
      goto cleanup;
    }
    if (limitless__rational_den_is_one(&r)) {
      tmp.kind = LIMITLESS_KIND_INT;
      st = limitless__bigint_copy(ctx, &tmp.v.i, &r.num);
      limitless__rational_clear(ctx, &r);
      if (st != LIMITLESS_OK) goto cleanup;
    } else {
      limitless__bigint_clear_raw(ctx, &tmp.v.i);
      tmp.kind = LIMITLESS_KIND_RAT;
      limitless__rational_init(&tmp.v.r);
      st = limitless__rational_copy(ctx, &tmp.v.r, &r);
      limitless__rational_clear(ctx, &r);
      if (st != LIMITLESS_OK) goto cleanup;
    }
  }

  limitless__number_swap(out, &tmp);
  st = LIMITLESS_OK;

cleanup:
  limitless_number_clear(ctx, &tmp);
  return st;
}
/* GCOVR_EXCL_STOP */

LIMITLESS_API limitless_status limitless_number_from_float_exact(limitless_ctx* ctx, limitless_number* out, float v) {
  limitless_u32 bits = 0u;
  const limitless_u8* src = (const limitless_u8*)&v;
  limitless_u8* dst = (limitless_u8*)&bits;
  limitless_u32 frac;
  int sign;
  int exp;
  limitless_u64 mant;
  int e2;
  dst[0] = src[0];
  dst[1] = src[1];
  dst[2] = src[2];
  dst[3] = src[3];

  sign = (bits >> 31) ? -1 : 1;
  exp = (int)((bits >> 23) & 0xffu);
  frac = bits & 0x7fffffu;

  if (exp == 0xff) return LIMITLESS_EINVAL;
  if (exp == 0) {
    if (frac == 0) return limitless_number_from_u64(ctx, out, 0);
    mant = (limitless_u64)frac;
    e2 = 1 - 127 - 23;
  } else {
    mant = (limitless_u64)((1u << 23) | frac);
    e2 = exp - 127 - 23;
  }
  return limitless__number_from_mantissa_exp2(ctx, out, sign, mant, e2);
}

LIMITLESS_API limitless_status limitless_number_from_double_exact(limitless_ctx* ctx, limitless_number* out, double v) {
  limitless_u64 bits = 0ULL;
  const limitless_u8* src = (const limitless_u8*)&v;
  limitless_u8* dst = (limitless_u8*)&bits;
  limitless_u64 frac;
  int sign;
  int exp;
  limitless_u64 mant;
  int e2;
  dst[0] = src[0];
  dst[1] = src[1];
  dst[2] = src[2];
  dst[3] = src[3];
  dst[4] = src[4];
  dst[5] = src[5];
  dst[6] = src[6];
  dst[7] = src[7];

  sign = (bits >> 63) ? -1 : 1;
  exp = (int)((bits >> 52) & 0x7ffULL);
  frac = bits & 0x000fffffffffffffULL;

  if (exp == 0x7ff) return LIMITLESS_EINVAL;
  if (exp == 0) {
    if (frac == 0ULL) return limitless_number_from_u64(ctx, out, 0);
    mant = frac;
    e2 = 1 - 1023 - 52;
  } else {
    mant = (1ULL << 52) | frac;
    e2 = exp - 1023 - 52;
  }
  return limitless__number_from_mantissa_exp2(ctx, out, sign, mant, e2);
}

LIMITLESS_API limitless_status limitless_number_from_cstr(limitless_ctx* ctx, limitless_number* out, const char* s, int base) {
  limitless_number tmp;
  limitless_status st;
  const char* slash;
  const char* end;
  const char* end2;
  if (!ctx || !out || !s) return LIMITLESS_EINVAL;

  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;

  slash = s;
  while (*slash && *slash != '/') ++slash;

  if (*slash == '\0') {
    tmp.kind = LIMITLESS_KIND_INT;
    st = limitless__bigint_from_base_digits(ctx, &tmp.v.i, s, base, &end);
    if (st != LIMITLESS_OK) goto cleanup;
    if (*end != '\0') {
      st = LIMITLESS_EPARSE;
      goto cleanup;
    }
  } else {
    tmp.kind = LIMITLESS_KIND_RAT;
    limitless__rational_init(&tmp.v.r);

    st = limitless__bigint_from_base_digits(ctx, &tmp.v.r.num, s, base, &end);
    if (st != LIMITLESS_OK) goto cleanup;
    if (end != slash) {
      st = LIMITLESS_EPARSE;
      goto cleanup;
    }

    st = limitless__bigint_from_base_digits(ctx, &tmp.v.r.den, slash + 1, base, &end2);
    if (st != LIMITLESS_OK) goto cleanup;
    if (*end2 != '\0') {
      st = LIMITLESS_EPARSE;
      goto cleanup;
    }
    st = limitless__rational_normalize(ctx, &tmp.v.r);
    if (st != LIMITLESS_OK) goto cleanup;

    if (limitless__rational_den_is_one(&tmp.v.r)) {
      limitless_bigint iv;
      limitless__bigint_init_raw(&iv);
      st = limitless__bigint_copy(ctx, &iv, &tmp.v.r.num);
      if (st != LIMITLESS_OK) {
        limitless__bigint_clear_raw(ctx, &iv);
        goto cleanup;
      }
      limitless__rational_clear(ctx, &tmp.v.r);
      tmp.kind = LIMITLESS_KIND_INT;
      tmp.v.i = iv;
    }
  }

  limitless__number_swap(out, &tmp);
  st = LIMITLESS_OK;

cleanup:
  limitless_number_clear(ctx, &tmp);
  return st;
}

LIMITLESS_API limitless_status limitless_number_from_str(limitless_ctx* ctx, limitless_number* out, const char* s) {
  return limitless_number_from_cstr(ctx, out, s, 0);
}

LIMITLESS_API limitless_status limitless_number_to_cstr(limitless_ctx* ctx, const limitless_number* n, int base, char* buf, limitless_size cap, limitless_size* written) {
  limitless_status st;
  char* a = NULL;
  char* b = NULL;
  limitless_size la = 0;
  limitless_size lb = 0;
  limitless_size need = 0;
  if (!ctx || !n || base < 2 || base > 36) return LIMITLESS_EINVAL;

  if (n->kind == LIMITLESS_KIND_INT) {
    st = limitless__bigint_to_base_string(ctx, &n->v.i, base, &a, &la);
    if (st != LIMITLESS_OK) return st;
    need = la;
    if (n->v.i.sign < 0 && (la == 0 || a[0] != '-')) {
      /* bigint printer prints abs only */
      char* s = (char*)limitless__alloc_bytes(ctx, la + 2);
      if (!s) {
        limitless__free_bytes(ctx, a, la + 1);
        return LIMITLESS_EOOM;
      }
      s[0] = '-';
      limitless__mem_copy(s + 1, a, la + 1);
      limitless__free_bytes(ctx, a, la + 1);
      a = s;
      la += 1;
      need = la;
    }
  } else if (n->kind == LIMITLESS_KIND_RAT) {
    st = limitless__bigint_to_base_string(ctx, &n->v.r.num, base, &a, &la);
    if (st != LIMITLESS_OK) return st;
    st = limitless__bigint_to_base_string(ctx, &n->v.r.den, base, &b, &lb);
    if (st != LIMITLESS_OK) {
      limitless__free_bytes(ctx, a, la + 1);
      return st;
    }
    if (n->v.r.num.sign < 0 && (la == 0 || a[0] != '-')) {
      char* s = (char*)limitless__alloc_bytes(ctx, la + 2);
      if (!s) {
        limitless__free_bytes(ctx, a, la + 1);
        limitless__free_bytes(ctx, b, lb + 1);
        return LIMITLESS_EOOM;
      }
      s[0] = '-';
      limitless__mem_copy(s + 1, a, la + 1);
      limitless__free_bytes(ctx, a, la + 1);
      a = s;
      la += 1;
    }
    need = la + 1 + lb;
  } else {
    return LIMITLESS_EINVAL;
  }

  if (written) *written = need;
  if (!buf || cap <= need) {
    if (a) limitless__free_bytes(ctx, a, la + 1);
    if (b) limitless__free_bytes(ctx, b, lb + 1);
    return LIMITLESS_EBUF;
  }

  if (n->kind == LIMITLESS_KIND_INT) {
    limitless__mem_copy(buf, a, la);
    buf[la] = '\0';
  } else {
    limitless__mem_copy(buf, a, la);
    buf[la] = '/';
    limitless__mem_copy(buf + la + 1, b, lb);
    buf[la + 1 + lb] = '\0';
  }

  if (a) limitless__free_bytes(ctx, a, la + 1);
  if (b) limitless__free_bytes(ctx, b, lb + 1);
  return LIMITLESS_OK;
}

LIMITLESS_API limitless_status limitless_number_to_str(limitless_ctx* ctx, const limitless_number* n, char* buf, limitless_size cap, limitless_size* written) {
  return limitless_number_to_cstr(ctx, n, 10, buf, cap, written);
}

LIMITLESS_API limitless_status limitless_number_to_u64(limitless_ctx* ctx, const limitless_number* n, limitless_u64* out) {
  const limitless_bigint* i;
  (void)ctx;
  if (!out) return LIMITLESS_EINVAL;
  if (limitless__number_get_integer_ref(n, &i) != LIMITLESS_OK) return LIMITLESS_ETYPE;
  if (i->sign < 0) return LIMITLESS_ERANGE;
  return limitless__bigint_abs_to_u64(i, out);
}

LIMITLESS_API limitless_status limitless_number_to_i64(limitless_ctx* ctx, const limitless_number* n, limitless_i64* out) {
  const limitless_bigint* i;
  limitless_u64 mag;
  limitless_status st;
  (void)ctx;
  if (!out) return LIMITLESS_EINVAL;
  if (limitless__number_get_integer_ref(n, &i) != LIMITLESS_OK) return LIMITLESS_ETYPE;
  st = limitless__bigint_abs_to_u64(i, &mag);
  if (st != LIMITLESS_OK) return st;
  if (i->sign >= 0) {
    if (mag > 0x7fffffffffffffffULL) return LIMITLESS_ERANGE;
    *out = (limitless_i64)mag;
  } else {
    if (mag > 0x8000000000000000ULL) return LIMITLESS_ERANGE;
    if (mag == 0x8000000000000000ULL) {
      *out = (limitless_i64)(-9223372036854775807LL - 1LL);
    } else {
      *out = -(limitless_i64)mag;
    }
  }
  return LIMITLESS_OK;
}

/* GCOVR_EXCL_START */
static limitless_status limitless__number_binop(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b, int op) {
  limitless_status st;
  limitless_number tmp;
  if (!ctx || !out || !a || !b) return LIMITLESS_EINVAL;
  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;

  if (a->kind == LIMITLESS_KIND_INT && b->kind == LIMITLESS_KIND_INT) {
    tmp.kind = LIMITLESS_KIND_INT;
    if (op == 0) st = limitless__bigint_add_signed(ctx, &tmp.v.i, &a->v.i, &b->v.i);
    else if (op == 1) st = limitless__bigint_sub_signed(ctx, &tmp.v.i, &a->v.i, &b->v.i);
    else st = limitless__bigint_mul_signed(ctx, &tmp.v.i, &a->v.i, &b->v.i);
  } else {
    limitless_rational ra, rb, rr;
    limitless_bigint t1, t2;
    limitless__rational_init(&ra);
    limitless__rational_init(&rb);
    limitless__rational_init(&rr);
    limitless__bigint_init_raw(&t1);
    limitless__bigint_init_raw(&t2);

    st = limitless__number_to_rational_copy(ctx, &ra, a); if (st != LIMITLESS_OK) goto rat_cleanup;
    st = limitless__number_to_rational_copy(ctx, &rb, b); if (st != LIMITLESS_OK) goto rat_cleanup;

    if (op == 0 || op == 1) {
      st = limitless__bigint_mul_signed(ctx, &t1, &ra.num, &rb.den); if (st != LIMITLESS_OK) goto rat_cleanup;
      st = limitless__bigint_mul_signed(ctx, &t2, &rb.num, &ra.den); if (st != LIMITLESS_OK) goto rat_cleanup;
      if (op == 0) st = limitless__bigint_add_signed(ctx, &rr.num, &t1, &t2);
      else st = limitless__bigint_sub_signed(ctx, &rr.num, &t1, &t2);
      if (st != LIMITLESS_OK) goto rat_cleanup;
      st = limitless__bigint_mul_signed(ctx, &rr.den, &ra.den, &rb.den); if (st != LIMITLESS_OK) goto rat_cleanup;
    } else {
      st = limitless__bigint_mul_signed(ctx, &rr.num, &ra.num, &rb.num); if (st != LIMITLESS_OK) goto rat_cleanup;
      st = limitless__bigint_mul_signed(ctx, &rr.den, &ra.den, &rb.den); if (st != LIMITLESS_OK) goto rat_cleanup;
    }

    st = limitless__rational_normalize(ctx, &rr);
    if (st != LIMITLESS_OK) goto rat_cleanup;

    if (limitless__rational_den_is_one(&rr)) {
      tmp.kind = LIMITLESS_KIND_INT;
      st = limitless__bigint_copy(ctx, &tmp.v.i, &rr.num);
    } else {
      tmp.kind = LIMITLESS_KIND_RAT;
      limitless__rational_init(&tmp.v.r);
      st = limitless__rational_copy(ctx, &tmp.v.r, &rr);
    }

rat_cleanup:
    limitless__bigint_clear_raw(ctx, &t1);
    limitless__bigint_clear_raw(ctx, &t2);
    limitless__rational_clear(ctx, &ra);
    limitless__rational_clear(ctx, &rb);
    limitless__rational_clear(ctx, &rr);
  }

  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    return st;
  }

  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);
  return LIMITLESS_OK;
}
/* GCOVR_EXCL_STOP */

LIMITLESS_API limitless_status limitless_number_add(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b) {
  return limitless__number_binop(ctx, out, a, b, 0);
}

LIMITLESS_API limitless_status limitless_number_sub(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b) {
  return limitless__number_binop(ctx, out, a, b, 1);
}

LIMITLESS_API limitless_status limitless_number_mul(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b) {
  return limitless__number_binop(ctx, out, a, b, 2);
}

LIMITLESS_API limitless_status limitless_number_div(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b) {
  limitless_status st;
  limitless_number tmp;
  if (!ctx || !out || !a || !b) return LIMITLESS_EINVAL;
  if (limitless_number_is_zero(b)) return LIMITLESS_EDIVZERO;

  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;

  if (a->kind == LIMITLESS_KIND_INT && b->kind == LIMITLESS_KIND_INT) {
    limitless_bigint q, r;
    limitless__bigint_init_raw(&q);
    limitless__bigint_init_raw(&r);
    st = limitless__bigint_divmod_signed(ctx, &q, &r, &a->v.i, &b->v.i);
    if (st != LIMITLESS_OK) {
      limitless__bigint_clear_raw(ctx, &q);
      limitless__bigint_clear_raw(ctx, &r);
      limitless_number_clear(ctx, &tmp);
      return st;
    }
    if (r.used == 0) {
      tmp.kind = LIMITLESS_KIND_INT;
      st = limitless__bigint_copy(ctx, &tmp.v.i, &q);
    } else {
      limitless_rational rr;
      limitless__rational_init(&rr);
      st = limitless__bigint_copy(ctx, &rr.num, &a->v.i);
      if (st == LIMITLESS_OK) st = limitless__bigint_copy(ctx, &rr.den, &b->v.i);
      if (st == LIMITLESS_OK) st = limitless__rational_normalize(ctx, &rr);
      if (st == LIMITLESS_OK) {
        tmp.kind = LIMITLESS_KIND_RAT;
        limitless__rational_init(&tmp.v.r);
        st = limitless__rational_copy(ctx, &tmp.v.r, &rr);
        limitless__rational_clear(ctx, &rr);
      } else {
        limitless__rational_clear(ctx, &rr);
      }
    }
    limitless__bigint_clear_raw(ctx, &q);
    limitless__bigint_clear_raw(ctx, &r);
  } else {
    limitless_rational ra, rb, rr;
    limitless__rational_init(&ra);
    limitless__rational_init(&rb);
    limitless__rational_init(&rr);

    st = limitless__number_to_rational_copy(ctx, &ra, a); if (st != LIMITLESS_OK) goto rat_done;
    st = limitless__number_to_rational_copy(ctx, &rb, b); if (st != LIMITLESS_OK) goto rat_done;

    st = limitless__bigint_mul_signed(ctx, &rr.num, &ra.num, &rb.den); if (st != LIMITLESS_OK) goto rat_done;
    st = limitless__bigint_mul_signed(ctx, &rr.den, &ra.den, &rb.num); if (st != LIMITLESS_OK) goto rat_done;
    st = limitless__rational_normalize(ctx, &rr); if (st != LIMITLESS_OK) goto rat_done;

    if (limitless__rational_den_is_one(&rr)) {
      tmp.kind = LIMITLESS_KIND_INT;
      st = limitless__bigint_copy(ctx, &tmp.v.i, &rr.num);
    } else {
      tmp.kind = LIMITLESS_KIND_RAT;
      limitless__rational_init(&tmp.v.r);
      st = limitless__rational_copy(ctx, &tmp.v.r, &rr);
    }

rat_done:
    limitless__rational_clear(ctx, &ra);
    limitless__rational_clear(ctx, &rb);
    limitless__rational_clear(ctx, &rr);
  }

  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    return st;
  }

  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);
  return LIMITLESS_OK;
}

LIMITLESS_API limitless_status limitless_number_neg(limitless_ctx* ctx, limitless_number* out, const limitless_number* a) {
  limitless_number tmp;
  limitless_status st;
  if (!ctx || !out || !a) return LIMITLESS_EINVAL;
  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;
  st = limitless_number_copy(ctx, &tmp, a);
  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    return st;
  }
  if (tmp.kind == LIMITLESS_KIND_INT) {
    tmp.v.i.sign = -tmp.v.i.sign;
  } else {
    tmp.v.r.num.sign = -tmp.v.r.num.sign;
  }
  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);
  return LIMITLESS_OK;
}

LIMITLESS_API limitless_status limitless_number_abs(limitless_ctx* ctx, limitless_number* out, const limitless_number* a) {
  limitless_number tmp;
  limitless_status st;
  if (!ctx || !out || !a) return LIMITLESS_EINVAL;
  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;
  st = limitless_number_copy(ctx, &tmp, a);
  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    return st;
  }
  if (tmp.kind == LIMITLESS_KIND_INT) {
    if (tmp.v.i.sign < 0) tmp.v.i.sign = 1;
  } else {
    if (tmp.v.r.num.sign < 0) tmp.v.r.num.sign = 1;
  }
  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);
  return LIMITLESS_OK;
}

LIMITLESS_API int limitless_number_cmp(limitless_ctx* ctx, const limitless_number* a, const limitless_number* b, limitless_status* st) {
  if (st) *st = LIMITLESS_OK;
  if (!ctx || !a || !b) {
    if (st) *st = LIMITLESS_EINVAL;
    return 0;
  }

  if (a->kind == LIMITLESS_KIND_INT && b->kind == LIMITLESS_KIND_INT) {
    return limitless__bigint_cmp_signed(&a->v.i, &b->v.i);
  }

  {
    limitless_rational ra, rb;
    limitless_bigint l, r;
    limitless_status s;
    int cmp;
    limitless__rational_init(&ra); limitless__rational_init(&rb);
    limitless__bigint_init_raw(&l); limitless__bigint_init_raw(&r);

    s = limitless__number_to_rational_copy(ctx, &ra, a);
    if (s != LIMITLESS_OK) goto fail;
    s = limitless__number_to_rational_copy(ctx, &rb, b);
    if (s != LIMITLESS_OK) goto fail;

    s = limitless__bigint_mul_signed(ctx, &l, &ra.num, &rb.den);
    if (s != LIMITLESS_OK) goto fail;
    s = limitless__bigint_mul_signed(ctx, &r, &rb.num, &ra.den);
    if (s != LIMITLESS_OK) goto fail;

    cmp = limitless__bigint_cmp_signed(&l, &r);
    limitless__rational_clear(ctx, &ra);
    limitless__rational_clear(ctx, &rb);
    limitless__bigint_clear_raw(ctx, &l);
    limitless__bigint_clear_raw(ctx, &r);
    return cmp;

fail:
    if (st) *st = s;
    limitless__rational_clear(ctx, &ra);
    limitless__rational_clear(ctx, &rb);
    limitless__bigint_clear_raw(ctx, &l);
    limitless__bigint_clear_raw(ctx, &r);
    return 0;
  }
}

LIMITLESS_API limitless_status limitless_number_gcd(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, const limitless_number* b) {
  const limitless_bigint* ia;
  const limitless_bigint* ib;
  limitless_number tmp;
  limitless_status st;
  if (!ctx || !out || !a || !b) return LIMITLESS_EINVAL;
  if (limitless__number_get_integer_ref(a, &ia) != LIMITLESS_OK) return LIMITLESS_ETYPE;
  if (limitless__number_get_integer_ref(b, &ib) != LIMITLESS_OK) return LIMITLESS_ETYPE;

  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) return st;
  tmp.kind = LIMITLESS_KIND_INT;
  st = limitless__bigint_gcd(ctx, &tmp.v.i, ia, ib);
  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    return st;
  }
  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);
  return LIMITLESS_OK;
}

LIMITLESS_API limitless_status limitless_number_pow_u64(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, limitless_u64 exp) {
  const limitless_bigint* ia;
  limitless_bigint base, res, t;
  limitless_status st;
  limitless_number tmp;

  if (!ctx || !out || !a) return LIMITLESS_EINVAL;
  if (limitless__number_get_integer_ref(a, &ia) != LIMITLESS_OK) return LIMITLESS_ETYPE;

  limitless__bigint_init_raw(&base);
  limitless__bigint_init_raw(&res);
  limitless__bigint_init_raw(&t);
  st = limitless__bigint_copy(ctx, &base, ia); if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__bigint_set_u64(ctx, &res, 1); if (st != LIMITLESS_OK) goto cleanup;

  while (exp > 0ULL) {
    if (exp & 1ULL) {
      st = limitless__bigint_mul_signed(ctx, &t, &res, &base);
      if (st != LIMITLESS_OK) goto cleanup;
      limitless__bigint_swap(&res, &t);
      t.used = 0; t.sign = 0;
    }
    exp >>= 1;
    if (exp == 0ULL) break;
    st = limitless__bigint_mul_signed(ctx, &t, &base, &base);
    if (st != LIMITLESS_OK) goto cleanup;
    limitless__bigint_swap(&base, &t);
    t.used = 0; t.sign = 0;
  }

  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) goto cleanup;
  tmp.kind = LIMITLESS_KIND_INT;
  st = limitless__bigint_copy(ctx, &tmp.v.i, &res);
  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    goto cleanup;
  }
  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);

cleanup:
  limitless__bigint_clear_raw(ctx, &base);
  limitless__bigint_clear_raw(ctx, &res);
  limitless__bigint_clear_raw(ctx, &t);
  return st;
}

LIMITLESS_API limitless_status limitless_number_modexp_u64(limitless_ctx* ctx, limitless_number* out, const limitless_number* a, limitless_u64 exp, const limitless_number* mod) {
  const limitless_bigint* ia;
  const limitless_bigint* im;
  limitless_bigint base, res, t, m;
  limitless_status st;
  limitless_number tmp;

  if (!ctx || !out || !a || !mod) return LIMITLESS_EINVAL;
  if (limitless__number_get_integer_ref(a, &ia) != LIMITLESS_OK) return LIMITLESS_ETYPE;
  if (limitless__number_get_integer_ref(mod, &im) != LIMITLESS_OK) return LIMITLESS_ETYPE;
  if (im->sign <= 0 || im->used == 0) return LIMITLESS_EDIVZERO;

  limitless__bigint_init_raw(&base);
  limitless__bigint_init_raw(&res);
  limitless__bigint_init_raw(&t);
  limitless__bigint_init_raw(&m);

  st = limitless__bigint_copy(ctx, &m, im); if (st != LIMITLESS_OK) goto cleanup;
  if (m.sign < 0) m.sign = 1;

  st = limitless__bigint_mod(ctx, &base, ia, &m); if (st != LIMITLESS_OK) goto cleanup;
  if (base.sign < 0) {
    st = limitless__bigint_add_signed(ctx, &base, &base, &m);
    if (st != LIMITLESS_OK) goto cleanup;
  }

  st = limitless__bigint_set_u64(ctx, &res, 1);
  if (st != LIMITLESS_OK) goto cleanup;
  st = limitless__bigint_mod(ctx, &t, &res, &m);
  if (st != LIMITLESS_OK) goto cleanup;
  limitless__bigint_swap(&res, &t);
  t.used = 0; t.sign = 0;

  while (exp > 0ULL) {
    if (exp & 1ULL) {
      st = limitless__bigint_mul_signed(ctx, &t, &res, &base);
      if (st != LIMITLESS_OK) goto cleanup;
      st = limitless__bigint_mod(ctx, &res, &t, &m);
      if (st != LIMITLESS_OK) goto cleanup;
      t.used = 0; t.sign = 0;
    }
    exp >>= 1;
    if (exp == 0ULL) break;
    st = limitless__bigint_mul_signed(ctx, &t, &base, &base);
    if (st != LIMITLESS_OK) goto cleanup;
    st = limitless__bigint_mod(ctx, &base, &t, &m);
    if (st != LIMITLESS_OK) goto cleanup;
    t.used = 0; t.sign = 0;
  }

  st = limitless_number_init(ctx, &tmp);
  if (st != LIMITLESS_OK) goto cleanup;
  tmp.kind = LIMITLESS_KIND_INT;
  st = limitless__bigint_copy(ctx, &tmp.v.i, &res);
  if (st != LIMITLESS_OK) {
    limitless_number_clear(ctx, &tmp);
    goto cleanup;
  }
  limitless__number_swap(out, &tmp);
  limitless_number_clear(ctx, &tmp);

cleanup:
  limitless__bigint_clear_raw(ctx, &base);
  limitless__bigint_clear_raw(ctx, &res);
  limitless__bigint_clear_raw(ctx, &t);
  limitless__bigint_clear_raw(ctx, &m);
  return st;
}

LIMITLESS_API int limitless_number_is_zero(const limitless_number* n) {
  if (!n) return 1;
  if (n->kind == LIMITLESS_KIND_INT) return n->v.i.used == 0;
  if (n->kind == LIMITLESS_KIND_RAT) return n->v.r.num.used == 0;
  return 1;
}

LIMITLESS_API int limitless_number_is_integer(const limitless_number* n) {
  if (!n) return 0;
  if (n->kind == LIMITLESS_KIND_INT) return 1;
  if (n->kind == LIMITLESS_KIND_RAT) return limitless__rational_den_is_one(&n->v.r);
  return 0;
}

LIMITLESS_API int limitless_number_sign(const limitless_number* n) {
  if (!n) return 0;
  if (n->kind == LIMITLESS_KIND_INT) return n->v.i.sign;
  if (n->kind == LIMITLESS_KIND_RAT) return n->v.r.num.sign;
  return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* LIMITLESS_IMPLEMENTATION */

#endif /* LIMITLESS_H */
