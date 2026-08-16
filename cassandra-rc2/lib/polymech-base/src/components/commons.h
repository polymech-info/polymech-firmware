#ifndef COMMONS_H
#define COMMONS_H

#include <functional>
#include <type_traits>

/**
 * @class DummyFeature
 * @brief An empty class used as a base when a feature is disabled.
 */
class NoopFeature
{
};

// Helper to select a base class based on a boolean flag
template <bool B, class T>
using maybe = typename std::conditional<B, T, NoopFeature>::type;


#endif // COMMONS_H