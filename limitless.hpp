/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef LIMITLESS_HPP
#define LIMITLESS_HPP

#ifndef __cplusplus
#error "limitless.hpp requires C++"
#endif

#include <string>
#include <type_traits>

#ifdef LIMITLESS_H
#error "include limitless.hpp before limitless.h in C++ files"
#endif

#ifndef LIMITLESS_CPP_LEGACY_API
#define LIMITLESS_CPP_LEGACY_API 1
#endif

#ifndef LIMITLESS_CPP_DEPRECATED_FN
#if defined(_MSC_VER)
#define LIMITLESS_CPP_DEPRECATED_FN(msg) __declspec(deprecated(msg))
#elif defined(__clang__) || defined(__GNUC__)
#define LIMITLESS_CPP_DEPRECATED_FN(msg) __attribute__((deprecated(msg)))
#else
#define LIMITLESS_CPP_DEPRECATED_FN(msg)
#endif
#endif

#if defined(LIMITLESS_CPP_NO_LEGACY_DEPRECATION)
#undef LIMITLESS_CPP_DEPRECATED_FN
#define LIMITLESS_CPP_DEPRECATED_FN(msg)
#endif

#define limitless_number limitless_c_number
#include "limitless.h"
#undef limitless_number

namespace limitless {

/*
TU-coherence note:
These helpers intentionally use external inline linkage (not inline static).
That ensures one per-thread wrapper state across translation units.
*/
inline limitless_status& limitless_cpp__last_status_ref(void) {
  static thread_local limitless_status st = LIMITLESS_OK;
  return st;
}

inline void limitless_cpp__set_last_status(limitless_status st) {
  limitless_cpp__last_status_ref() = st;
}

inline void limitless_cpp__zero_raw(limitless_c_number* n) {
  n->kind = LIMITLESS_KIND_INT;
  n->v.i.sign = 0;
  n->v.i.used = 0;
  n->v.i.cap = 0;
  n->v.i.limbs = NULL;
}

inline limitless_ctx*& limitless_cpp__ctx_override_ref(void) {
  static thread_local limitless_ctx* p = NULL;
  return p;
}

inline limitless_ctx* limitless_cpp__builtin_ctx(void) {
  struct limitless_cpp__ctx_holder {
    limitless_ctx ctx;
    limitless_status st;
    limitless_cpp__ctx_holder() : st(limitless_ctx_init_default(&ctx)) {}
  };
  static limitless_cpp__ctx_holder holder; /* GCOVR_EXCL_BR_LINE */
  limitless_cpp__set_last_status(holder.st);
  if (holder.st != LIMITLESS_OK) return NULL; /* GCOVR_EXCL_BR_LINE */
  return &holder.ctx;
}

inline limitless_ctx* limitless_cpp__active_ctx(void) {
  limitless_ctx* p = limitless_cpp__ctx_override_ref();
  if (p) return p;
  return limitless_cpp__builtin_ctx();
}

inline void limitless_cpp_set_default_ctx(limitless_ctx* ctx) {
  limitless_cpp__ctx_override_ref() = ctx;
}

inline limitless_ctx* limitless_cpp_get_default_ctx(void) {
  return limitless_cpp__active_ctx();
}

inline limitless_status limitless_cpp_last_status(void) {
  return limitless_cpp__last_status_ref();
}

class limitless_number {
public:
  limitless_number() : owner_ctx_(NULL) {
    limitless_cpp__zero_raw(&raw_);
    limitless_cpp__set_last_status(rebind_owner(limitless_cpp__active_ctx()));
  }

  limitless_number(limitless_i32 value) : limitless_number() { *this = value; } /* GCOVR_EXCL_BR_LINE */
  limitless_number(limitless_u32 value) : limitless_number() { *this = value; } /* GCOVR_EXCL_BR_LINE */
  limitless_number(limitless_i64 value) : limitless_number() { *this = value; } /* GCOVR_EXCL_BR_LINE */
  limitless_number(limitless_u64 value) : limitless_number() { *this = value; } /* GCOVR_EXCL_BR_LINE */
  limitless_number(float value) : limitless_number() { *this = value; } /* GCOVR_EXCL_BR_LINE */
  limitless_number(double value) : limitless_number() { *this = value; } /* GCOVR_EXCL_BR_LINE */

  limitless_number(const limitless_number& other) : owner_ctx_(NULL) {
    limitless_status st;
    limitless_ctx* ctx;
    limitless_cpp__zero_raw(&raw_);
    ctx = other.effective_ctx();
    st = rebind_owner(ctx);
    if (st == LIMITLESS_OK) { /* GCOVR_EXCL_BR_LINE */
      st = limitless_number_copy(ctx, &raw_, &other.raw_);
    }
    limitless_cpp__set_last_status(st);
  }

  limitless_number(limitless_number&& other) : owner_ctx_(other.owner_ctx_) {
    raw_ = other.raw_;
    limitless_cpp__zero_raw(&other.raw_);
    other.owner_ctx_ = NULL;
    limitless_cpp__set_last_status(LIMITLESS_OK);
  }

  ~limitless_number() {
    if (owner_ctx_) { /* GCOVR_EXCL_BR_LINE */
      limitless_number_clear(owner_ctx_, &raw_); /* GCOVR_EXCL_BR_LINE */
      owner_ctx_ = NULL;
      limitless_cpp__zero_raw(&raw_);
    }
  }

  limitless_number& operator=(const limitless_number& other) {
    limitless_status st;
    limitless_ctx* ctx;
    if (this == &other) { /* GCOVR_EXCL_BR_LINE */
      limitless_cpp__set_last_status(LIMITLESS_OK);
      return *this;
    }
    ctx = owner_ctx_ ? owner_ctx_ : other.effective_ctx(); /* GCOVR_EXCL_BR_LINE */
    st = rebind_owner(ctx);
    if (st == LIMITLESS_OK) { /* GCOVR_EXCL_BR_LINE */
      st = limitless_number_copy(ctx, &raw_, &other.raw_);
    }
    limitless_cpp__set_last_status(st);
    return *this;
  }

  limitless_number& operator=(limitless_number&& other) {
    if (this == &other) { /* GCOVR_EXCL_BR_LINE */
      limitless_cpp__set_last_status(LIMITLESS_OK);
      return *this;
    }
    if (owner_ctx_) {
      limitless_number_clear(owner_ctx_, &raw_);
    }
    raw_ = other.raw_;
    owner_ctx_ = other.owner_ctx_;
    limitless_cpp__zero_raw(&other.raw_);
    other.owner_ctx_ = NULL;
    limitless_cpp__set_last_status(LIMITLESS_OK);
    return *this;
  }

  limitless_number& operator=(limitless_i32 value) {
    return assign_i64((limitless_i64)value);
  }

  limitless_number& operator=(limitless_u32 value) {
    return assign_u64((limitless_u64)value);
  }

  limitless_number& operator=(limitless_i64 value) {
    return assign_i64(value);
  }

  limitless_number& operator=(limitless_u64 value) {
    return assign_u64(value);
  }

  limitless_number& operator=(float value) {
    limitless_ctx* ctx = effective_ctx();
    limitless_status st = ensure_owner(ctx);
    if (st == LIMITLESS_OK) {
      st = limitless_number_from_float_exact(ctx, &raw_, value);
    }
    limitless_cpp__set_last_status(st);
    return *this;
  }

  limitless_number& operator=(double value) {
    limitless_ctx* ctx = effective_ctx();
    limitless_status st = ensure_owner(ctx);
    if (st == LIMITLESS_OK) { /* GCOVR_EXCL_BR_LINE */
      st = limitless_number_from_double_exact(ctx, &raw_, value);
    }
    limitless_cpp__set_last_status(st);
    return *this;
  }

  limitless_number operator+(const limitless_number& rhs) const {
    return binary_op(rhs, limitless_number_add);
  }

  limitless_number operator-(const limitless_number& rhs) const {
    return binary_op(rhs, limitless_number_sub);
  }

  limitless_number operator*(const limitless_number& rhs) const {
    return binary_op(rhs, limitless_number_mul);
  }

  limitless_number operator/(const limitless_number& rhs) const {
    return binary_op(rhs, limitless_number_div);
  }

  limitless_number operator-() const {
    limitless_number out;
    limitless_ctx* ctx = effective_ctx();
    limitless_status st = out.rebind_owner(ctx);
    if (st == LIMITLESS_OK) {
      st = limitless_number_neg(ctx, &out.raw_, &raw_);
    }
    limitless_cpp__set_last_status(st);
    return out;
  }

  limitless_number& operator+=(const limitless_number& rhs) {
    return binary_op_inplace(rhs, limitless_number_add);
  }

  limitless_number& operator-=(const limitless_number& rhs) {
    return binary_op_inplace(rhs, limitless_number_sub);
  }

  limitless_number& operator*=(const limitless_number& rhs) {
    return binary_op_inplace(rhs, limitless_number_mul);
  }

  limitless_number& operator/=(const limitless_number& rhs) {
    return binary_op_inplace(rhs, limitless_number_div);
  }

  bool operator==(const limitless_number& rhs) const {
    return cmp(rhs) == 0;
  }

  bool operator!=(const limitless_number& rhs) const {
    return cmp(rhs) != 0;
  }

  bool operator<(const limitless_number& rhs) const {
    return cmp(rhs) < 0;
  }

  bool operator<=(const limitless_number& rhs) const {
    return cmp(rhs) <= 0;
  }

  bool operator>(const limitless_number& rhs) const {
    return cmp(rhs) > 0;
  }

  bool operator>=(const limitless_number& rhs) const {
    return cmp(rhs) >= 0;
  }

  static limitless_number parse(const char* s, int base = 0) {
    limitless_number out;
    limitless_ctx* ctx = out.effective_ctx();
    limitless_status st = out.ensure_owner(ctx);
    if (st == LIMITLESS_OK) {
      st = limitless_number_from_cstr(ctx, &out.raw_, s, base);
    }
    limitless_cpp__set_last_status(st);
    return out; /* GCOVR_EXCL_BR_LINE */
  } /* GCOVR_EXCL_BR_LINE */

  std::string str(int base = 10) const {
    std::string out;
    limitless_ctx* ctx = effective_ctx();
    limitless_status st;
    limitless_size needed = 0;
    if (!ctx) { /* GCOVR_EXCL_BR_LINE */
      limitless_cpp__set_last_status(LIMITLESS_EINVAL); /* GCOVR_EXCL_BR_LINE */
      return out;
    }
    st = limitless_number_to_cstr(ctx, &raw_, base, NULL, 0, &needed);
    if (st != LIMITLESS_EBUF && st != LIMITLESS_OK) { /* GCOVR_EXCL_BR_LINE */
      limitless_cpp__set_last_status(st); /* GCOVR_EXCL_BR_LINE */
      return out;
    }
    out.resize((size_t)(needed + 1));
    st = limitless_number_to_cstr(ctx, &raw_, base, &out[0], needed + 1, NULL);
    if (st != LIMITLESS_OK) { /* GCOVR_EXCL_BR_LINE */
      limitless_cpp__set_last_status(st); /* GCOVR_EXCL_BR_LINE */
      return std::string();
    }
    out.resize((size_t)needed);
    limitless_cpp__set_last_status(LIMITLESS_OK);
    return out;
  }

  const limitless_c_number* raw() const { return &raw_; }
  limitless_c_number* raw() { return &raw_; }

private:
  typedef limitless_status (*binary_fn)(limitless_ctx*, limitless_c_number*, const limitless_c_number*, const limitless_c_number*);

  limitless_ctx* effective_ctx() const {
    limitless_ctx* ctx = owner_ctx_ ? owner_ctx_ : limitless_cpp__active_ctx(); /* GCOVR_EXCL_BR_LINE */
    return ctx;
  }

  limitless_status rebind_owner(limitless_ctx* ctx) {
    if (owner_ctx_ == ctx) return ctx ? LIMITLESS_OK : LIMITLESS_EINVAL;
    if (owner_ctx_) { /* GCOVR_EXCL_BR_LINE */
      limitless_number_clear(owner_ctx_, &raw_);
    }
    owner_ctx_ = NULL;
    limitless_cpp__zero_raw(&raw_);
    if (!ctx) return LIMITLESS_EINVAL; /* GCOVR_EXCL_BR_LINE */
    owner_ctx_ = ctx;
    return limitless_number_init(owner_ctx_, &raw_);
  }

  limitless_status ensure_owner(limitless_ctx* ctx) {
    if (!owner_ctx_) { /* GCOVR_EXCL_BR_LINE */
      return rebind_owner(ctx);
    }
    return LIMITLESS_OK;
  }

  limitless_number& assign_i64(limitless_i64 value) {
    limitless_ctx* ctx = effective_ctx();
    limitless_status st = ensure_owner(ctx);
    if (st == LIMITLESS_OK) {
      st = limitless_number_from_i64(ctx, &raw_, value);
    }
    limitless_cpp__set_last_status(st);
    return *this;
  }

  limitless_number& assign_u64(limitless_u64 value) {
    limitless_ctx* ctx = effective_ctx();
    limitless_status st = ensure_owner(ctx);
    if (st == LIMITLESS_OK) { /* GCOVR_EXCL_BR_LINE */
      st = limitless_number_from_u64(ctx, &raw_, value);
    }
    limitless_cpp__set_last_status(st);
    return *this;
  }

  limitless_number binary_op(const limitless_number& rhs, binary_fn fn) const {
    limitless_number out;
    limitless_ctx* ctx = owner_ctx_ ? owner_ctx_ : effective_ctx(); /* GCOVR_EXCL_BR_LINE */
    limitless_status st = out.rebind_owner(ctx);
    if (st == LIMITLESS_OK) { /* GCOVR_EXCL_BR_LINE */
      st = fn(ctx, &out.raw_, &raw_, &rhs.raw_);
    }
    limitless_cpp__set_last_status(st);
    return out;
  } /* GCOVR_EXCL_BR_LINE */

  limitless_number& binary_op_inplace(const limitless_number& rhs, binary_fn fn) {
    limitless_ctx* ctx = effective_ctx();
    limitless_status st = ensure_owner(ctx);
    if (st == LIMITLESS_OK) { /* GCOVR_EXCL_BR_LINE */
      st = fn(ctx, &raw_, &raw_, &rhs.raw_);
    }
    limitless_cpp__set_last_status(st);
    return *this;
  }

  int cmp(const limitless_number& rhs) const {
    limitless_status st = LIMITLESS_OK;
    limitless_ctx* ctx = effective_ctx();
    int v;
    if (!ctx) { /* GCOVR_EXCL_BR_LINE */
      limitless_cpp__set_last_status(LIMITLESS_EINVAL);
      return 0;
    }
    v = limitless_number_cmp(ctx, &raw_, &rhs.raw_, &st);
    limitless_cpp__set_last_status(st);
    if (st != LIMITLESS_OK) return 0; /* GCOVR_EXCL_BR_LINE */
    return v;
  }

  limitless_c_number raw_;
  limitless_ctx* owner_ctx_;
};

/* Preferred namespaced type alias for C++ users. */
using number = limitless_number;

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, limitless_number>::type
operator+(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) + rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, limitless_number>::type
operator-(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) - rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, limitless_number>::type
operator*(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) * rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, limitless_number>::type
operator/(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) / rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
operator==(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) == rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
operator!=(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) != rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
operator<(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) < rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
operator<=(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) <= rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
operator>(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) > rhs; /* GCOVR_EXCL_BR_LINE */
}

template <typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value, bool>::type
operator>=(T lhs, const limitless_number& rhs) {
  return limitless_number(lhs) >= rhs; /* GCOVR_EXCL_BR_LINE */
}

} /* namespace limitless */

/*
Compatibility layer:
Keep legacy global C++ symbols available while preferring namespace usage.
Target removal for legacy global names: v0.3.0.
*/
#if LIMITLESS_CPP_LEGACY_API
typedef ::limitless::number limitless_number;

LIMITLESS_CPP_DEPRECATED_FN("use limitless::limitless_cpp_set_default_ctx (legacy global wrapper scheduled for removal in v0.3.0)")
inline void limitless_cpp_set_default_ctx(limitless_ctx* ctx) {
  ::limitless::limitless_cpp_set_default_ctx(ctx);
}

LIMITLESS_CPP_DEPRECATED_FN("use limitless::limitless_cpp_get_default_ctx (legacy global wrapper scheduled for removal in v0.3.0)")
inline limitless_ctx* limitless_cpp_get_default_ctx(void) {
  return ::limitless::limitless_cpp_get_default_ctx();
}

LIMITLESS_CPP_DEPRECATED_FN("use limitless::limitless_cpp_last_status (legacy global wrapper scheduled for removal in v0.3.0)")
inline limitless_status limitless_cpp_last_status(void) {
  return ::limitless::limitless_cpp_last_status();
}
#endif

#endif /* LIMITLESS_HPP */
