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
