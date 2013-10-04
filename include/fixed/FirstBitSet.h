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

#ifndef FIXED_FIRST_BIT_SET_H
#define FIXED_FIRST_BIT_SET_H

#include "fixed/Absolute.h"

#include <cstdint>
#include <type_traits>

namespace fixed {

//
// NOTE
//   The values returned range from 0 to 64 for 64bit values, and 0 to 128 for
//   128 bit values.  The value of 0 means no bit was actually set, and 1 means
//   the 1st (lowest) bit, with 64 and 128 representing the max bit (highest)
//   respectively.
//
class FirstBitSet {
  public:
    unsigned int operator() (const int32_t val) const noexcept;
    unsigned int operator() (const uint32_t val) const noexcept;
    unsigned int operator() (const int64_t val) const noexcept;
    unsigned int operator() (const uint64_t val) const noexcept;
    unsigned int operator() (const __int128_t val) const noexcept;
    unsigned int operator() (const __uint128_t val) const noexcept;

    //
    // These two are provided for compile time support, they should not be used
    // by runtime code, will be much slower!!
    //
    static constexpr unsigned int findConstExpr (int64_t val) noexcept;

    static constexpr unsigned int findConstExpr (
        uint64_t val,
        uint64_t mask = static_cast<uint64_t> (1) << 63,
        unsigned int bitPos = 64
    ) noexcept;

    //
    // Note, by making this constexpr, uses of this call will resolve to a
    // compile time constant.
    //
    template <typename T>
    static constexpr unsigned int maxBitPos () noexcept;

  private:

    static unsigned int bsr (const uint32_t val) noexcept;
    static unsigned int bsr (const uint64_t val) noexcept;
    static unsigned int bsr (const __uint128_t val) noexcept;

    static constexpr uint32_t ONES_32 = 0xFFFFFFFF;

    static constexpr uint64_t UPPER32_MASK_64 =
        static_cast<uint64_t> (ONES_32) << 32;

    static constexpr uint64_t ONES_64 = UPPER32_MASK_64 | ONES_32;

    static constexpr __uint128_t UPPER64_MASK_128 =
        static_cast<__uint128_t> (ONES_64) << 64;
};

template <typename T>
inline constexpr unsigned int FirstBitSet::maxBitPos () noexcept
{
    static_assert (
        std::numeric_limits<__int128_t>::is_specialized,
        "Need to compile with -std=c++11 -U__STRICT_ANSI__ "
        "in order to get __int128_t type_trait and numeric_limits support"
    );

    return (
        std::is_signed<T>::value ?
            (sizeof (T) * 8 - 1) : (sizeof (T) * 8)
    );
}

inline unsigned int FirstBitSet::operator() (const int32_t val) const noexcept
{
    return bsr (absoluteValue<uint32_t> (val));
}

inline unsigned int FirstBitSet::operator() (const uint32_t val) const noexcept
{
    return bsr (val);
}

inline unsigned int FirstBitSet::operator() (const int64_t val) const noexcept
{
    return bsr (absoluteValue<uint64_t> (val));
}

inline unsigned int FirstBitSet::operator() (const uint64_t val) const noexcept
{
    return bsr (val);
}

inline unsigned int
FirstBitSet::operator() (const __int128_t val) const noexcept
{
    return bsr (absoluteValue<__uint128_t> (val));
}

inline unsigned int
FirstBitSet::operator() (const __uint128_t val) const noexcept
{
    return bsr (val);
}

//
// Note, the asm call 'bsr' means 'Bit Scan Reverse'.  It will find the first
// bit set in the integer.  Or written a different way, it will find the
// position of the most significant bit set.
//
// Returns a value from 0-32.
//   A return value of 0 means no bits were set to 1.
//   A return value of 1 means the least significant bit and a value
//   of 32 refers to the most significant bit.
//
inline unsigned int FirstBitSet::bsr (const uint32_t val) noexcept
{
    //
    // The asm call is not defined for a value of 0
    //
    if (! val)
    {
        return 0;
    }

    int bitset;

    asm (
        "movl %1, %%eax;"
        "bsr %%eax, %0;"
        : "=r" (bitset)
        : "r" (val)
        : "%eax"
    );

    //
    // The asm call numbers from 0 - 31, we want 1 - 32, so add one.
    //
    return bitset + 1;
}

inline unsigned int FirstBitSet::bsr (const uint64_t val) noexcept
{
    if (UPPER32_MASK_64 & val)
    {
        return 32 + bsr (static_cast<uint32_t> (val >> 32));
    }
    else
    {
        return bsr (static_cast<uint32_t> (val));
    }
}

inline unsigned int FirstBitSet::bsr (const __uint128_t val) noexcept
{
    if (UPPER64_MASK_128 & val)
    {
        return 64 + bsr (static_cast<uint64_t> (val >> 64));
    }
    else
    {
        return bsr (static_cast<uint64_t> (val));
    }
}

constexpr unsigned int FirstBitSet::findConstExpr (int64_t val) noexcept
{
    return findConstExpr (absoluteValue<uint64_t> (val));
}

constexpr unsigned int FirstBitSet::findConstExpr (
    uint64_t val,
    uint64_t mask,
    unsigned int bitPos
) noexcept
{
    return (
        (! bitPos || (mask & val)) ?
            bitPos : findConstExpr (val, mask >> 1, bitPos - 1)
    );
}

} // namespace fixed

#endif // FIXED_FIRST_BIT_SET_H
