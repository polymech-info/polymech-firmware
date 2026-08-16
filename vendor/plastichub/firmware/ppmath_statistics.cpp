#include "ppmath_statistics.h"

Statistic::Statistic()
{
    clear();
}

// resets all counters
void Statistic::clear()
{
    _cnt = 0;
    _sum = 0;
    _min = 0;
    _max = 0;
#ifdef STAT_USE_STDEV
    _ssqdif = 0.0; // not _ssq but sum of square differences
    // which is SUM(from i = 1 to N) of
    // (f(i)-_ave_N)**2
#endif
}

// adds a new value to the data-set
void Statistic::add(const float value)
{
    if (_cnt == 0)
    {
        _min = value;
        _max = value;
    }
    else
    {
        if (value < _min)
            _min = value;
        else if (value > _max)
            _max = value;
    }
    _sum += value;
    _cnt++;

#ifdef STAT_USE_STDEV
    if (_cnt > 1)
    {
        float _store = (_sum / _cnt - value);
        _ssqdif = _ssqdif + _cnt * _store * _store / (_cnt - 1);
        // ~10% faster but limits the amount of samples to 65K as _cnt*_cnt overflows
        // float _store = _sum - _cnt * value;
        // _ssqdif = _ssqdif + _store * _store / (_cnt*_cnt - _cnt);
    }
#endif
}

// returns the average of the data-set added sofar
float Statistic::average() const
{
    if (_cnt == 0)
        return NAN; // original code returned 0
    return _sum / _cnt;
}

// Population standard deviation = s = sqrt [ S ( Xi - � )2 / N ]
// http://www.suite101.com/content/how-is-standard-deviation-used-a99084
#ifdef STAT_USE_STDEV

float Statistic::variance() const
{
    if (_cnt == 0)
        return NAN; // otherwise DIV0 error
    return _ssqdif / _cnt;
}

float Statistic::mean() const
{
    if (_cnt == 0)
        return NAN; // otherwise DIV0 error
    return this->sum() / _cnt;
}

float Statistic::pop_stdev() const
{
    if (_cnt == 0)
        return NAN; // otherwise DIV0 error
    return sqrt(_ssqdif / _cnt);
}

float Statistic::unbiased_stdev() const
{
    if (_cnt < 2)
        return NAN; // otherwise DIV0 error
    return sqrt(_ssqdif / (_cnt - 1));
}

#endif
// END OF FILE