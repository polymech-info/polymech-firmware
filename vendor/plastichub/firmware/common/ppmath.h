#ifndef PPMATH_H
#define PPMATH_H

template <typename T>
T clamp(const T &value, const T &low, const T &high)
{
  return value < low ? low : (value > high ? high : value);
}

#define RANGE(i, min, max) ((i > min) && (i < max)) ? true : false

#define NCLAMP(x, min, max) (x - min) / (max - min)

#endif