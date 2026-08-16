#ifndef PPMATH_STATISTICS_H
#define PPMATH_STATISTICS_H

#include <Arduino.h>
#include <math.h>

#define STAT_USE_STDEV

#ifdef __cplusplus

// C++11 solution that is standards compliant. Return type is deduced automatically
template <class L, class R>
static inline constexpr auto MIN(const L lhs, const R rhs) -> decltype(lhs + rhs)
{
  return lhs < rhs ? lhs : rhs;
}
template <class L, class R>
static inline constexpr auto MAX(const L lhs, const R rhs) -> decltype(lhs + rhs)
{
  return lhs > rhs ? lhs : rhs;
}
template <class T>
static inline constexpr const T ABS(const T v)
{
  return v >= 0 ? v : -v;
}
#else
// Using GCC extensions, but Travis GCC version does not like it and gives
//  "error: statement-expressions are not allowed outside functions nor in template-argument lists"
#define MIN(a, b) \
  ({__typeof__(a) _a = (a); \
      __typeof__(b) _b = (b); \
      _a < _b ? _a : _b; })

#define MAX(a, b) \
  ({__typeof__(a) _a = (a); \
      __typeof__(b) _b = (b); \
      _a > _b ? _a : _b; })

#define ABS(a) \
  ({__typeof__(a) _a = (a); \
      _a >= 0 ? _a : -_a; })

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

/**
 * Returns the kth q-quantile.
 * @link http://en.wikipedia.org/wiki/Quantile#Quantiles_of_a_population
 * ie: median is 1st 2-quantile
 * ie: upper quartile is 3rd 4-quantile
 * @return {Number} q-quantile of values.
 */
/*

const quantile = (arr: number[], i: number, n: number) => {
    if (i === 0) return Math.min.apply(null, arr);
    if (i === n) return Math.max.apply(null, arr);

    let sorted = arr.slice(0);
    sorted.sort((a, b) => a - b);
    let index = sorted.length * i / n;

    if (index % 1 === 0) {
        return 0.5 * sorted[index - 1] + 0.5 * sorted[index];
    }

    return sorted[~~index];
};

export const median = (arr: number[]) => quantile(arr, 1, 2);

export const sum = (arr: number[]) => arr.reduce((a, b) => a + b, 0);

export const mean = (arr: number[]) => sum(arr) / arr.length;

// sqare errors along mean
const sdiff = (arr: number[], mean: number) => arr.map((v) =>    Math.pow(v - mean, 2));
export const standardDeviation = (arr: number[]) => Math.sqrt(mean(sdiff(arr, mean(arr))));
*/

#endif