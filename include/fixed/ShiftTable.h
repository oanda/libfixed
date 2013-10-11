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

#ifndef FIXED_SHIFT_TABLE_H
#define FIXED_SHIFT_TABLE_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

#include "fixed/Absolute.h"
#include "fixed/FirstBitSet.h"

namespace fixed {

//
// Note, this stuff belongs in FixedNumber, but it was messy, so isolating
// it here, but is really meant only for FixedNumber
//

template <typename T>
class ShiftTable {
  public:
    ShiftTable (uint64_t maxIntegerValue);

    struct ShiftValue {
        ShiftValue (unsigned int decimalPlaces, uint64_t maxIntegerValue);

        T computePow10 (unsigned int exp);

        unsigned int decimalPlaces;
        T value;
        T halfRangeVal;
        unsigned int firstBitSet;
        __int128_t integerOverflowCheckValPos;
        __int128_t integerOverflowCheckValNeg;
    };

    const ShiftValue& operator[] (int idx) const;

    //
    // Throws fixed::BadValueException if value wasn't found
    //
    const ShiftValue& find_if (
        const std::function <bool (const ShiftValue& sv)>& func
    ) const;

    //
    // Throws fixed::BadValueException if value wasn't found
    //
    const ShiftValue& find_if_not (
        const std::function <bool (const ShiftValue& sv)>& func
    ) const;

    unsigned int totalDigitsOfPrecision (const T& value) const noexcept;

    unsigned int integerDigitsOfPrecision (
        const T& value,
        unsigned int currentDecimalPlace
    ) const noexcept;

    static_assert (
        std::is_same<T, int64_t>::value || std::is_same<T, __int128_t>::value,
        "This class is only intended to work with int164 or int128"
    );

    static_assert (
        std::numeric_limits<__int128_t>::is_specialized,
        "Need to compile with -std=c++11 -U__STRICT_ANSI__ "
        "in order to get __int128_t type_trait and numeric_limits support"
    );

    static constexpr unsigned int MAX_DIGITS = (
        std::is_same<T, int64_t>::value ?
            std::numeric_limits<int64_t>::digits10 :
            std::numeric_limits<__int128_t>::digits10
    );

  private:

    std::vector<ShiftValue> table_;

};

template <typename T>
ShiftTable<T>::ShiftTable (uint64_t maxIntegerValue)
{
    for (unsigned int i = 0; i < (MAX_DIGITS + 1); ++i)
    {
        table_.push_back (ShiftValue (i, maxIntegerValue));
    }
}

template <typename T>
const typename ShiftTable<T>::ShiftValue& ShiftTable<T>::find_if (
    const std::function <bool (const ShiftValue& sv)>& func
) const
{
    const auto iter = std::find_if (
        table_.begin (),
        table_.end (),
        func
    );

    //
    // Note, it really should not be possible for us to hit this error, it
    // means something must be wrong elsewhere for us to need to find a
    // firstBitSet that is larger than any in the shift table
    //
    if (iter == table_.end ())
    {
        throw fixed::BadValueException (
            "ShiftTable find_if failed, max decimal places: " +
            std::to_string (MAX_DIGITS)
        );
    }

    return *iter;
}

template <typename T>
const typename ShiftTable<T>::ShiftValue& ShiftTable<T>::find_if_not (
    const std::function <bool (const ShiftValue& sv)>& func
) const
{
    const auto iter = std::find_if_not (
        table_.begin (),
        table_.end (),
        func
    );

    //
    // Note, it really should not be possible for us to hit this error, it
    // means something must be wrong elsewhere for us to need to find a
    // firstBitSet that is larger than any in the shift table
    //
    if (iter == table_.end ())
    {
        throw fixed::BadValueException (
            "ShiftTable find_if_not failed, max digits: " +
            std::to_string (MAX_DIGITS)
        );
    }

    return *iter;
}

template <typename T>
unsigned int ShiftTable<T>::totalDigitsOfPrecision (
    const T& value
) const noexcept
{
    static_assert (
        std::is_same<T, int64_t>::value || std::is_same<T, __int128_t>::value,
        "This class is only intended to work with int164 or int128"
    );

    T absVal = absoluteValue<T> (value);

    const auto& sv =
        this->find_if (
            [&] (const ShiftValue& sv) { return absVal < sv.value; }
        );

    return sv.decimalPlaces;
}

template <typename T>
unsigned int ShiftTable<T>::integerDigitsOfPrecision (
    const T& value,
    unsigned int currentDecimalPlace
) const noexcept
{
    auto digitsOfPrecision = totalDigitsOfPrecision (value);

    return (
        digitsOfPrecision > currentDecimalPlace?
            (digitsOfPrecision - currentDecimalPlace) : 0
    );
}

template <typename T>
T ShiftTable<T>::ShiftValue::computePow10 (unsigned int exp)
{
    T val = 1;

    for (int i = 0; i < exp; ++i)
    {
        val *= 10;
    }

    return val;
}

template <typename T>
ShiftTable<T>::ShiftValue::ShiftValue (
    unsigned int dps, // decimalPlaces
    uint64_t maxIntegerValue
)
{
    decimalPlaces = dps;
    value = computePow10 (dps);

    halfRangeVal = value / 2;

    firstBitSet = dps == 0 ? 0 : FirstBitSet () (value);

    integerOverflowCheckValPos =
        static_cast<__int128_t> (maxIntegerValue) * value + value - 1;

    integerOverflowCheckValNeg = 0 - integerOverflowCheckValPos;
}

template <typename T>
inline const typename ShiftTable<T>::ShiftValue&
ShiftTable<T>::operator[] (int idx) const
{
    return table_.at (idx);
}

} // namespace fixed

#endif // FIXED_SHIFT_TABLE_H
