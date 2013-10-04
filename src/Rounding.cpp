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

#include "fixed/Rounding.h"

#include <cassert>

namespace fixed {

const std::vector<std::string> Rounding::modeStrings_ = {
  {
    "DOWN",
    "UP",
    "TOWARDS_ZERO",
    "AWAY_FROM_ZERO",
    "TO_NEAREST_HALF_UP",
    "TO_NEAREST_HALF_DOWN",
    "TO_NEAREST_HALF_AWAY_FROM_ZERO",
    "TO_NEAREST_HALF_TOWARDS_ZERO",
    "TO_NEAREST_HALF_TO_EVEN",
    "TO_NEAREST_HALF_TO_ODD"
  }
};

const Rounding::RunTimeModeStringsCheck Rounding::runTimeModeStringsCheck_;

template <typename T>
static T roundDownAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T&, // halfRangeVal
    const bool negativeFlag
)
{
    if (negativeFlag && decimalVal)
    {
        return -1;
    }

    return 0;
}

template <typename T>
static T roundUpAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T&, // halfRangeVal
    const bool negativeFlag
)
{
    if (! negativeFlag && decimalVal)
    {
        return 1;
    }

    return 0;
}

template <typename T>
static T roundTowardsZeroAdjustment (
    const T&, // integerVal
    const T&, // decimalVal
    const T&, // halfRangeVal
    const bool // negativeFlag
)
{
    return 0;
}

template <typename T>
static T roundAwayFromZeroAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T&, // halfRangeVal
    const bool negativeFlag
)
{
    if (decimalVal)
    {
        return negativeFlag ? -1 : 1;
    }

    return 0;
}

template <typename T>
static T roundToNearestHalfUpAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T& halfRangeVal,
    const bool negativeFlag
)
{
    if (negativeFlag)
    {
        if (decimalVal > halfRangeVal)
        {
            return -1;
        }
    }
    else
    {
        if (decimalVal >= halfRangeVal)
        {
            return 1;
        }
    }

    return 0;
}

template <typename T>
static T roundToNearestHalfDownAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T& halfRangeVal,
    const bool negativeFlag
)
{
    if (negativeFlag)
    {
        if (decimalVal >= halfRangeVal)
        {
            return -1;
        }
    }
    else
    {
        if (decimalVal > halfRangeVal)
        {
            return 1;
        }
    }

    return 0;
}

template <typename T>
static T roundToNearestHalfAwayFromZeroAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T& halfRangeVal,
    const bool negativeFlag
)
{
    if (negativeFlag)
    {
        if (decimalVal >= halfRangeVal)
        {
            return -1;
        }
    }
    else
    {
        if (decimalVal >= halfRangeVal)
        {
            return 1;
        }
    }

    return 0;
}

template <typename T>
static T roundToNearestHalfTowardsZeroAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T& halfRangeVal,
    const bool negativeFlag
)
{
    if (negativeFlag)
    {
        if (decimalVal > halfRangeVal)
        {
            return -1;
        }
    }
    else
    {
        if (decimalVal > halfRangeVal)
        {
            return 1;
        }
    }

    return 0;
}

template <typename T>
static T roundToNearestHalfToEvenAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T& halfRangeVal,
    const bool negativeFlag
)
{
    const bool odd = integerVal & 0x1;

    if (negativeFlag)
    {
        if (odd)
        {
            if (decimalVal >= halfRangeVal)
            {
                return -1;
            }
        }
        else // even
        {
            if (decimalVal > halfRangeVal)
            {
                return -1;
            }
        }
    }
    else
    {
        if (odd)
        {
            if (decimalVal >= halfRangeVal)
            {
                return 1;
            }
        }
        else // even
        {
            if (decimalVal > halfRangeVal)
            {
                return 1;
            }
        }
    }

    return 0;
}

template <typename T>
static T roundToNearestHalfToOddAdjustment (
    const T& integerVal,
    const T& decimalVal,
    const T& halfRangeVal,
    const bool negativeFlag
)
{
    const bool odd = integerVal & 0x1;

    if (negativeFlag)
    {
        if (odd)
        {
            if (decimalVal > halfRangeVal)
            {
                return -1;
            }
        }
        else // even
        {
            if (decimalVal >= halfRangeVal)
            {
                return -1;
            }
        }
    }
    else
    {
        if (odd)
        {
            if (decimalVal > halfRangeVal)
            {
                return 1;
            }
        }
        else // even
        {
            if (decimalVal >= halfRangeVal)
            {
                return 1;
            }
        }
    }

    return 0;
}

const std::vector<
    std::function<
        int64_t (
            const int64_t& integerVal,
            const int64_t& decimalVal,
            const int64_t& halfRangeVal,
            const bool negativeFlag
        )
    >
> Rounding::roundingAdjustments64_ = {
    roundDownAdjustment<int64_t>,
    roundUpAdjustment<int64_t>,
    roundTowardsZeroAdjustment<int64_t>,
    roundAwayFromZeroAdjustment<int64_t>,
    roundToNearestHalfUpAdjustment<int64_t>,
    roundToNearestHalfDownAdjustment<int64_t>,
    roundToNearestHalfAwayFromZeroAdjustment<int64_t>,
    roundToNearestHalfTowardsZeroAdjustment<int64_t>,
    roundToNearestHalfToEvenAdjustment<int64_t>,
    roundToNearestHalfToOddAdjustment<int64_t>
};

const std::vector<
    std::function<
        __int128_t (
            const __int128_t& integerVal,
            const __int128_t& decimalVal,
            const __int128_t& halfRangeVal,
            const bool negativeFlag
        )
    >
> Rounding::roundingAdjustments128_ = {
    roundDownAdjustment<__int128_t>,
    roundUpAdjustment<__int128_t>,
    roundTowardsZeroAdjustment<__int128_t>,
    roundAwayFromZeroAdjustment<__int128_t>,
    roundToNearestHalfUpAdjustment<__int128_t>,
    roundToNearestHalfDownAdjustment<__int128_t>,
    roundToNearestHalfAwayFromZeroAdjustment<__int128_t>,
    roundToNearestHalfTowardsZeroAdjustment<__int128_t>,
    roundToNearestHalfToEvenAdjustment<__int128_t>,
    roundToNearestHalfToOddAdjustment<__int128_t>
};

Rounding::RunTimeModeStringsCheck::RunTimeModeStringsCheck ()
{
    size_t expectedSize =
        static_cast<size_t> (
            static_cast<std::underlying_type<Mode>::type> (
                Mode::MODE_MAX_VAL
            )
        );

    assert (expectedSize == modeStrings_.size ());
}

} // namespace fixed
