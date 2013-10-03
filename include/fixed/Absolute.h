#ifndef FIXED_ABSOLUTE_H
#define FIXED_ABSOLUTE_H

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <type_traits>

namespace fixed {

template <typename R, typename V>
inline constexpr R ABS_OF_MIN_VAL ()
{
    return (
        static_cast<R> (1) << (sizeof (V) * 8 - 1)
    );
}

//
// Callers of this should be aware that passing in the minimum value of a
// signed type and having the return type be that same signed type is undefined
// as that signed type can't hold the positive value.
//
template <typename R, typename V>
inline constexpr R absoluteValue (const V& value)
{
    static_assert (
        (sizeof (R) > sizeof (V)) ||
        ((sizeof (R) == sizeof (V) && (
            std::is_unsigned<R>::value || std::is_signed<V>::value
        ))),
        "Can't cast down in size of type"
    );

    return (value > 0) ?
            static_cast<R> (value) :
            value == std::numeric_limits<V>::min () ?
                static_cast<R> (ABS_OF_MIN_VAL<R, V> ()) :
                static_cast<R> (0 - value)
    ;
}

} // namespace fixed

#endif // FIXED_ABSOLUTE_H
