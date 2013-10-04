//
// The MIT License (MIT)
//
//
// Copyright (c) 2013 OANDA Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//

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
// Note, passing in the minimum value of a signed type and having the return
// type be that same signed type is undefined as that signed type can't hold
// the positive value.
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

    return (std::is_unsigned<V>::value || (value >= 0)) ?
            static_cast<R> (value) :
            value == std::numeric_limits<V>::min () ?
                static_cast<R> (ABS_OF_MIN_VAL<R, V> ()) :
                static_cast<R> (0 - value)
    ;
}

} // namespace fixed

#endif // FIXED_ABSOLUTE_H
