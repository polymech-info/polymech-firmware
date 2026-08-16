#ifndef MACROS_H
#define MACROS_H

#include "./xtypes.h"

// Macros for adding
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INCREMENT_(n) INC_##n
#define INCREMENT(n) INCREMENT_(n)

// Macros for subtracting
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC_6 5
#define DEC_7 6
#define DEC_8 7
#define DEC_9 8
#define DECREMENT_(n) DEC_##n
#define DECREMENT(n) DECREMENT_(n)

// compiler - & C quirks
#ifndef FORCE_INLINE
  #define FORCE_INLINE __attribute__((always_inline)) inline
#endif

#define _UNUSED __attribute__((unused))

// fallback noop
#define NOOP (void(0))

// Option testing

// Use NUM_ARGS(__VA_ARGS__) to get the number of variadic arguments
#define _NUM_ARGS(_, n, m, l, k, j, i, h, g, f, e, d, c, b, a, Z, Y, X, W, V, U, T, S, R, Q, P, O, N, M, L, K, J, I, H, G, F, E, D, C, B, A, OUT, ...) OUT
#define NUM_ARGS(V...) _NUM_ARGS(0, V, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Use TWO_ARGS(__VA_ARGS__) to get whether there are 1, 2, or >2 arguments
#define _TWO_ARGS(_, n, m, l, k, j, i, h, g, f, e, d, c, b, a, Z, Y, X, W, V, U, T, S, R, Q, P, O, N, M, L, K, J, I, H, G, F, E, D, C, B, A, OUT, ...) OUT
#define TWO_ARGS(V...) _TWO_ARGS(0, V, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0)

// Concatenate symbol names, without or with pre-expansion
#define _CAT(a, V...) a##V
#define CAT(a, V...) _CAT(a, V)

// Macros to chain up to 40 conditions
#define _DO_1(W, C, A) (_##W##_1(A))
#define _DO_2(W, C, A, B) (_##W##_1(A) C _##W##_1(B))
#define _DO_3(W, C, A, V...) (_##W##_1(A) C _DO_2(W, C, V))
#define _DO_4(W, C, A, V...) (_##W##_1(A) C _DO_3(W, C, V))
#define _DO_5(W, C, A, V...) (_##W##_1(A) C _DO_4(W, C, V))
#define _DO_6(W, C, A, V...) (_##W##_1(A) C _DO_5(W, C, V))
#define _DO_7(W, C, A, V...) (_##W##_1(A) C _DO_6(W, C, V))
#define _DO_8(W, C, A, V...) (_##W##_1(A) C _DO_7(W, C, V))
#define _DO_9(W, C, A, V...) (_##W##_1(A) C _DO_8(W, C, V))
#define _DO_10(W, C, A, V...) (_##W##_1(A) C _DO_9(W, C, V))
#define _DO_11(W, C, A, V...) (_##W##_1(A) C _DO_10(W, C, V))
#define _DO_12(W, C, A, V...) (_##W##_1(A) C _DO_11(W, C, V))
#define _DO_13(W, C, A, V...) (_##W##_1(A) C _DO_12(W, C, V))
#define _DO_14(W, C, A, V...) (_##W##_1(A) C _DO_13(W, C, V))
#define _DO_15(W, C, A, V...) (_##W##_1(A) C _DO_14(W, C, V))
#define _DO_16(W, C, A, V...) (_##W##_1(A) C _DO_15(W, C, V))
#define _DO_17(W, C, A, V...) (_##W##_1(A) C _DO_16(W, C, V))
#define _DO_18(W, C, A, V...) (_##W##_1(A) C _DO_17(W, C, V))
#define _DO_19(W, C, A, V...) (_##W##_1(A) C _DO_18(W, C, V))
#define _DO_20(W, C, A, V...) (_##W##_1(A) C _DO_19(W, C, V))
#define _DO_21(W, C, A, V...) (_##W##_1(A) C _DO_20(W, C, V))
#define _DO_22(W, C, A, V...) (_##W##_1(A) C _DO_21(W, C, V))
#define _DO_23(W, C, A, V...) (_##W##_1(A) C _DO_22(W, C, V))
#define _DO_24(W, C, A, V...) (_##W##_1(A) C _DO_23(W, C, V))
#define _DO_25(W, C, A, V...) (_##W##_1(A) C _DO_24(W, C, V))
#define _DO_26(W, C, A, V...) (_##W##_1(A) C _DO_25(W, C, V))
#define _DO_27(W, C, A, V...) (_##W##_1(A) C _DO_26(W, C, V))
#define _DO_28(W, C, A, V...) (_##W##_1(A) C _DO_27(W, C, V))
#define _DO_29(W, C, A, V...) (_##W##_1(A) C _DO_28(W, C, V))
#define _DO_30(W, C, A, V...) (_##W##_1(A) C _DO_29(W, C, V))
#define _DO_31(W, C, A, V...) (_##W##_1(A) C _DO_30(W, C, V))
#define _DO_32(W, C, A, V...) (_##W##_1(A) C _DO_31(W, C, V))
#define _DO_33(W, C, A, V...) (_##W##_1(A) C _DO_32(W, C, V))
#define _DO_34(W, C, A, V...) (_##W##_1(A) C _DO_33(W, C, V))
#define _DO_35(W, C, A, V...) (_##W##_1(A) C _DO_34(W, C, V))
#define _DO_36(W, C, A, V...) (_##W##_1(A) C _DO_35(W, C, V))
#define _DO_37(W, C, A, V...) (_##W##_1(A) C _DO_36(W, C, V))
#define _DO_38(W, C, A, V...) (_##W##_1(A) C _DO_37(W, C, V))
#define _DO_39(W, C, A, V...) (_##W##_1(A) C _DO_38(W, C, V))
#define _DO_40(W, C, A, V...) (_##W##_1(A) C _DO_39(W, C, V))
#define __DO_N(W, C, N, V...) _DO_##N(W, C, V)
#define _DO_N(W, C, N, V...) __DO_N(W, C, N, V)
#define DO(W, C, V...) (_DO_N(W, C, NUM_ARGS(V), V))

#define FIRST(a,...)     a
#define SECOND(a,b,...)  b
#define THIRD(a,b,c,...) c

#define IS_PROBE(V...) SECOND(V, 0)     // Get the second item passed, or 0
#define PROBE() ~, 1                    // Second item will be 1 if this is passed
#define _NOT_0 PROBE()
#define NOT(x) IS_PROBE(_CAT(_NOT_, x)) //   NOT('0') gets '1'. Anything else gets '0'.
#define _BOOL(x) NOT(NOT(x))            // _BOOL('0') gets '0'. Anything else gets '1'.

#define IF_ELSE(TF) _IF_ELSE(_BOOL(TF))
#define _IF_ELSE(TF) _CAT(_IF_, TF)

#define _IF_1(V...) V _IF_1_ELSE
#define _IF_0(...)    _IF_0_ELSE

#define _IF_1_ELSE(...)
#define _IF_0_ELSE(V...) V

// Recognize "true" values: blank, 1, 0x1, true
#define _ISENA_ ~, 1
#define _ISENA_1 ~, 1
#define _ISENA_0x1 ~, 1
#define _ISENA_true ~, 1
#define _ISENA(V...) IS_PROBE(V)

// Macros to evaluate simple option switches
#define _ENA_1(O) _ISENA(CAT(_IS, CAT(ENA_, O)))
#define _DIS_1(O) NOT(_ENA_1(O))
#define ENABLED(V...) DO(ENA, &&, V)

#ifndef MACRO_DISABLED
#define MACRO_DISABLED(V...) DO(DIS, &&, V)
#endif

#define ANY(V...) !MACRO_DISABLED(V)
#define ALL ENABLED
#define NONE MACRO_DISABLED
#define COUNT_ENABLED(V...) DO(ENA, +, V)
#define MANY(V...) (COUNT_ENABLED(V) > 1)

// #define _CAT(a, ...) a ## __VA_ARGS__
// #define SWITCH_ENABLED_      1
// #define ENABLED(b) _CAT(SWITCH_ENABLED_, b)

// time
#define PENDING(NOW, SOON) ((long)(NOW - (SOON)) < 0)
#define ELAPSED(NOW, SOON) (!PENDING(NOW, SOON))

#define MMM_TO_MMS(MM_M) ((MM_M) / 60.0f)
#define MMS_TO_MMM(MM_S) ((MM_S) * 60.0f)
#define HOUR_MS ((millis_t)1000 * (millis_t)(60 * 60))
#define MIN_MS ((millis_t)1000 * (millis_t)(60))
#define SECS ((millis_t)1000)

// bit masks
#undef __BV
#define __BV(b) (1 << (b))
#define TEST(n, b) !!((n) & __BV(b))
#define SBI(n, b) (n |= __BV(b))
#define CBI(n, b) (n &= ~__BV(b))
#define SET_BIT_TO(N, B, TF) \
  do                         \
  {                          \
    if (TF)                  \
      SBI(N, B);             \
    else                     \
      CBI(N, B);             \
  } while (0)

// Bitmask operations
#define TEST_MASK(n, b) !!((n) & (b))
#define SBI_MASK(n, b) (n |= (b))
#define CBI_MASK(n, b) (n &= ~(b))

#define _BV32(b) (1UL << (b))
#define TEST32(n, b) !!((n) & _BV32(b))
#define SBI32(n, b) (n |= _BV32(b))
#define CBI32(n, b) (n &= ~_BV32(b))
#define SIGN(a) ((a > 0) - (a < 0))

// math basics

#ifndef WITHIN
#define WITHIN(V, L, H) ((V) >= (L) && (V) <= (H))
#endif

#define NUMERIC(a) WITHIN(a, '0', '9')
#define DECIMAL(a) (NUMERIC(a) || a == '.')
#define NUMERIC_SIGNED(a) (NUMERIC(a) || (a) == '-' || (a) == '+')
#define DECIMAL_SIGNED(a) (DECIMAL(a) || (a) == '-' || (a) == '+')
#define COUNT(a) (sizeof(a) / sizeof(*a))

#ifndef ZERO
#define ZERO(a) memset(a, 0, sizeof(a))
#endif

#define COPY(a, b) memcpy(a, b, MIN(sizeof(a), sizeof(b)))

// #define M_PI 3.14159265358979323846f
#define RADIANS(d) ((d) * M_PI / 180.0f)
#define DEGREES(r) ((r) * 180.0f / M_PI)
#define CEILING(x, y) (((x) + (y) - 1) / (y))

#define UNEAR_ZERO(x) ((x) < 0.000001f)
#define NEAR_ZERO(x) WITHIN(x, -0.000001f, 0.000001f)
#define NEAR(x, y) NEAR_ZERO((x) - (y))

#define RECIPROCAL(x) (NEAR_ZERO(x) ? 0 : (1 / float(x)))
#define FIXFLOAT(f) ({__typeof__(f) _f = (f); _f + (_f < 0 ? -0.0000005f : 0.0000005f); })

// value helper macros
#define ISEOL(C) ((C) == '\n' || (C) == '\r')
#define HEXCHR(a) (NUMERIC(a) ? (a) - '0' : WITHIN(a, 'a', 'f') ? ((a) - 'a' + 10) \
                                        : WITHIN(a, 'A', 'F')   ? ((a) - 'A' + 10) \
                                                                : -1)

// Macros for initializing arrays
#define ARRAY_6(v1, v2, v3, v4, v5, v6, ...) \
  {                                          \
    v1, v2, v3, v4, v5, v6                   \
  }
#define ARRAY_5(v1, v2, v3, v4, v5, ...) \
  {                                      \
    v1, v2, v3, v4, v5                   \
  }
#define ARRAY_4(v1, v2, v3, v4, ...) \
  {                                  \
    v1, v2, v3, v4                   \
  }
#define ARRAY_3(v1, v2, v3, ...) \
  {                              \
    v1, v2, v3                   \
  }
#define ARRAY_2(v1, v2, ...) \
  {                          \
    v1, v2                   \
  }
#define ARRAY_1(v1, ...) \
  {                      \
    v1                   \
  }

#define _ARRAY_N(N, ...) ARRAY_##N(__VA_ARGS__)
#define ARRAY_N(N, ...) _ARRAY_N(N, __VA_ARGS__)

#define SPACE(A) " " << A << " "

////////////////////////////////////////////////////////////////////////
//
// Macros from Marlin / compat

#ifndef MARLIN_HEX_VERSION

// Clock speed factors
#if !defined(CYCLES_PER_MICROSECOND) && !defined(__STM32F1__)
#define CYCLES_PER_MICROSECOND (F_CPU / 1000000UL) // 16 or 20 on AVR
#endif

// Nanoseconds per cycle
#define NANOSECONDS_PER_CYCLE (1000000000.0 / F_CPU)

// Macros to make a string from a macro
#define STRINGIFY_(M) #M
#define STRINGIFY(M) STRINGIFY_(M)

// Macros to chain up to 40 conditions

#ifdef __cplusplus
#ifndef _MINMAX_H_
#define _MINMAX_H_

extern "C++"
{

  // C++11 solution that is standards compliant. Return type is deduced automatically
  template <class N>
  static constexpr N _MIN(const N val) { return val; }
  template <class N>
  static constexpr N _MAX(const N val) { return val; }
  template <class L, class R>
  static constexpr auto _MIN(const L lhs, const R rhs) -> decltype(lhs + rhs)
  {
    return lhs < rhs ? lhs : rhs;
  }
  template <class L, class R>
  static constexpr auto _MAX(const L lhs, const R rhs) -> decltype(lhs + rhs)
  {
    return lhs > rhs ? lhs : rhs;
  }
  template <class T, class... Ts>
  static constexpr const T _MIN(T V, Ts... Vs) { return _MIN(V, _MIN(Vs...)); }
  template <class T, class... Ts>
  static constexpr const T _MAX(T V, Ts... Vs) { return _MAX(V, _MAX(Vs...)); }
}

#endif

// Allow manipulating enumeration value like flags without ugly cast everywhere
#define ENUM_FLAGS(T)                                                                                                \
  FORCE_INLINE constexpr T operator&(T x, T y) { return static_cast<T>(static_cast<int>(x) & static_cast<int>(y)); } \
  FORCE_INLINE constexpr T operator|(T x, T y) { return static_cast<T>(static_cast<int>(x) | static_cast<int>(y)); } \
  FORCE_INLINE constexpr T operator^(T x, T y) { return static_cast<T>(static_cast<int>(x) ^ static_cast<int>(y)); } \
  FORCE_INLINE constexpr T operator~(T x) { return static_cast<T>(~static_cast<int>(x)); }                           \
  FORCE_INLINE T &operator&=(T &x, T y) { return x &= y; }                                                           \
  FORCE_INLINE T &operator|=(T &x, T y) { return x |= y; }                                                           \
  FORCE_INLINE T &operator^=(T &x, T y) { return x ^= y; }

// C++11 solution that is standard compliant. <type_traits> is not available on all platform
namespace Private
{
  template <bool, typename _Tp = void>
  struct enable_if
  {
  };
  template <typename _Tp>
  struct enable_if<true, _Tp>
  {
    typedef _Tp type;
  };

  template <typename T, typename U>
  struct is_same
  {
    enum
    {
      value = false
    };
  };
  template <typename T>
  struct is_same<T, T>
  {
    enum
    {
      value = true
    };
  };

  template <typename T, typename... Args>
  struct first_type_of
  {
    typedef T type;
  };
  template <typename T>
  struct first_type_of<T>
  {
    typedef T type;
  };
}
// C++11 solution using SFINAE to detect the existence of a member in a class at compile time.
// It creates a HasMember<Type> structure containing 'value' set to true if the member exists
#define HAS_MEMBER_IMPL(Member)                                       \
  namespace Private                                                   \
  {                                                                   \
    template <typename Type, typename Yes = char, typename No = long> \
    struct HasMember_##Member                                         \
    {                                                                 \
      template <typename C>                                           \
      static Yes &test(decltype(&C::Member));                         \
      template <typename C>                                           \
      static No &test(...);                                           \
      enum                                                            \
      {                                                               \
        value = sizeof(test<Type>(0)) == sizeof(Yes)                  \
      };                                                              \
    };                                                                \
  }

// Call the method if it exists, but do nothing if it does not. The method is detected at compile time.
// If the method exists, this is inlined and does not cost anything. Else, an "empty" wrapper is created, returning a default value
#define CALL_IF_EXISTS_IMPL(Return, Method, ...)                                                                                                                \
  HAS_MEMBER_IMPL(Method)                                                                                                                                       \
  namespace Private                                                                                                                                             \
  {                                                                                                                                                             \
    template <typename T, typename... Args>                                                                                                                     \
    FORCE_INLINE typename enable_if<HasMember_##Method<T>::value, Return>::type Call_##Method(T *t, Args... a) { return static_cast<Return>(t->Method(a...)); } \
    _UNUSED static Return Call_##Method(...) { return __VA_ARGS__; }                                                                                            \
  }
#define CALL_IF_EXISTS(Return, That, Method, ...) \
  static_cast<Return>(Private::Call_##Method(That, ##__VA_ARGS__))

#else

#endif // __cplusplus

#endif // MARLIN_HEX_VERSION

///////////////////////////////////////////////////////
//
//  String Conversion / Casting (WString.h substitute)
//
#define CC_STR(s) (const char *)s
#define C_STR(s) (char *)s

#endif // MACROS_H
