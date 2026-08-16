#ifndef XMATH_H
#define XMATH_H

#ifdef __cplusplus
extern "C++"
{
template <typename T>
  T clamp(const T &value, const T &low, const T &high)
  {
    return value < low ? low : (value > high ? high : value);
  }
}

#endif

#define RANGE(i, min, max) ((i > min) && (i < max))

#define NCLAMP(x, min, max) \
    (((max) == (min)) ? 0.0f : \
     clamp<float>((static_cast<float>((x) - (min))) / ((max) - (min)), 0.0f, 1.0f))


#endif