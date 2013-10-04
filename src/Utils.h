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

#ifndef FIXED_UTILS_H
#define FIXED_UTILS_H

#include "fixed/Absolute.h"

#include <algorithm>

namespace fixed {

std::string uint128ToStr (__uint128_t v, bool negative = false)
{
    char buf[64];

    int idx = 0;

    while (v)
    {
        buf[idx++] = '0' + v % 10;
        v /= 10;
    }

    if (negative)
    {
        buf[idx++] = '-';
    }

    std::reverse (buf, buf + idx);

    buf[idx] = '\0';

    return std::string (buf);
}

std::string int128ToStr (const __int128_t& v)
{
    return uint128ToStr (absoluteValue<__uint128_t> (v), v < 0);
}

} // namespace fixed

#endif // FIXED_UTILS_H
