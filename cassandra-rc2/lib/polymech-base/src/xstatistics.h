#ifndef XSTATISTICS_H
#define XSTATISTICS_H

#include <Arduino.h>
#include <math.h>

#define STAT_USE_STDEV

#ifdef __cplusplus

// C++11 solution that is standards compliant. Return type is deduced automatically
template <class L, class R>
static inline constexpr auto XMIN(const L lhs, const R rhs) -> decltype(lhs + rhs)
{
  return lhs < rhs ? lhs : rhs;
}
template <class L, class R>
static inline constexpr auto XMAX(const L lhs, const R rhs) -> decltype(lhs + rhs)
{
  return lhs > rhs ? lhs : rhs;
}
template <class T>

static inline constexpr const T XABS(const T v)
{
  return v >= 0 ? v : -v;
}
#else
// Using GCC extensions, but Travis GCC version does not like it and gives
//  "error: statement-expressions are not allowed outside functions nor in template-argument lists"
#define XMIN(a, b) \
  ({__typeof__(a) _a = (a); \
      __typeof__(b) _b = (b); \
      _a < _b ? _a : _b; })

#define XMAX(a, b) \
  ({__typeof__(a) _a = (a); \
      __typeof__(b) _b = (b); \
      _a > _b ? _a : _b; })

#define XABS(a) \
  ({__typeof__(a) _a = (a); \
      _a >= 0 ? _a : -_a; })

#endif

// Backward compatibility aliases for older ESP32 boards
#ifndef MIN
#define MIN XMIN
#endif
#ifndef MAX
#define MAX XMAX
#endif
#ifndef ABS
#define ABS XABS
#endif

class Statistic
{
public:
  Statistic();  // "switches on/off" stdev run time
  void clear(); // "switches on/off" stdev run time
  void add(const float);

  // returns the number of values added
  uint32_t count() const { return _cnt; }; // zero if empty
  float sum() const { return _sum; };      // zero if empty
  float minimum() const { return _min; };  // zero if empty
  float maximum() const { return _max; };  // zero if empty
  float average() const;                   // NAN if empty
  float mean() const;                      // zero if empty

#ifdef STAT_USE_STDEV
  float variance() const;       // NAN if empty
  float pop_stdev() const;      // population stdev  // NAN if empty
  float unbiased_stdev() const; // NAN if empty
#endif

protected:
  uint32_t _cnt;
  float _sum;
  float _min;
  float _max;
#ifdef STAT_USE_STDEV
  float _ssqdif; // sum of squares difference
#endif
};

#endif