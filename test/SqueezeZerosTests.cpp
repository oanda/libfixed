#include "fixed/FirstBitSet.h"
#include "TestsCommon.h"

#include <functional>
#include <iostream>
#include <stack>
#include <vector>

namespace fixed {
namespace test {

template <typename T>
unsigned int countZeros (T val)
{
    int cnt = 0;

    while ((val % 10) == 0)
    {
        cnt++;
        val /= 10;
    }

    return cnt;
}

template <typename S>
S& operator<< (S& s, __uint128_t v)
{
    std::stack<int> digits;

    while (v)
    {
        digits.push (v % 10);
        v /= 10;
    }

    while (! digits.empty ())
    {
        s << digits.top ();
        digits.pop ();
    }

    return s;
}

template <typename S>
S& operator<< (S& s, __int128_t v)
{
    if (v < 0)
    {
        s << "-";
        s << static_cast<__uint128_t> (0 - v);
    }
    else
    {
        s << static_cast<__uint128_t> (v);
    }

    return s;
}

template <typename T>
T computePow10 (int exp)
{
    if (exp == 0)
    {
        return 1;
    }

    T val = 1;

    for (int i = 0; i < exp; ++i)
    {
        val *= 10;
    }

    return val;
}

template <typename T, const T startVal>
bool test ()
{
    for (unsigned int maxSqueeze = 0; maxSqueeze < 40; ++maxSqueeze)
    {
        T curVal = startVal;

        while (true)
        {
            unsigned int numZeros = countZeros (curVal);

            T sval = curVal;

            unsigned int squeezed = Number::squeezeZeros (sval, maxSqueeze);

            unsigned int expectedSqueezed = std::min (maxSqueeze, numZeros);

            T expectedSqueezedVal = curVal / computePow10<T> (expectedSqueezed);

            if ((squeezed != expectedSqueezed) || (sval != expectedSqueezedVal))
            {
                std::cerr << "Expected num squeezed: " << expectedSqueezed
                          << " and val: " << expectedSqueezedVal
                          << " but got num squeezed: " << squeezed
                          << " and val: " << sval
                          << " cur val: " << curVal
                          << " max squeeze: " << maxSqueeze
                          << std::endl;

                return false;
            }

            T prevVal = curVal;
            curVal *= 10;

            if (((prevVal >= 0) && (curVal < prevVal)) ||
                ((prevVal < 0) && (curVal > prevVal)))
            {
                break;
            }
        }
    }

    return true;
}

std::vector<Test> NumberSqueezeZerosTestVec = {
    {test<int64_t, 1>, TestName ("<int64_t, 1>")},
    {test<int64_t, -1>, TestName ("<int64_t, -1>")},
    {test<int64_t, 10>, TestName ("<int64_t, 10>")},
    {test<int64_t, -10>, TestName ("<int64_t, -10>")},
    {test<int64_t, 123>, TestName ("<int64_t, 123>")},
    {test<int64_t, -123>, TestName ("<int64_t, -123>")},
    {test<int64_t, 123000>, TestName ("<int64_t, 123000>")},
    {test<int64_t, -123000>, TestName ("<int64_t, -123000>")},
    {test<__int128_t, 1>, TestName ("<int128_t, 1>")},
    {test<__int128_t, -1>, TestName ("<int128_t, -1>")},
    {test<__int128_t, 10>, TestName ("<int128_t, 10>")},
    {test<__int128_t, -10>, TestName ("<int128_t, -10>")},
    {test<__int128_t, 123>, TestName ("<int128_t, 123>")},
    {test<__int128_t, -123>, TestName ("<int128_t, -123>")},
    {test<__int128_t, 123000>, TestName ("<int128_t, 123000>")},
    {test<__int128_t, -123000>, TestName ("<int128_t, -123000>")}
};

} // namespace test
} // namespace fixed
