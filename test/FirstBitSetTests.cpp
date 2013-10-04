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

#include "fixed/FirstBitSet.h"
#include "TestsCommon.h"

#include <functional>
#include <iostream>
#include <vector>

namespace fixed {
namespace test {

template <
    typename T,
    unsigned int MAX_ITERATIONS,
    int startVal = 1,
    typename FBS
>
bool run_test (FBS fbs)
{
    T curVal = 0;

    for (unsigned int i = 0; i < MAX_ITERATIONS; ++i)
    {
        unsigned int got1 = fbs (curVal);
        unsigned int got2 = curVal > 0 ? fbs (curVal | 0x1) : got1;

        unsigned int got3 = curVal > 0 ? fbs (curVal | (curVal - 1)) : got1;

        if ((got1 != i) || (got2 != i) || (got3 != i))
        {
            std::cerr << "Expected: " << static_cast<int> (i)
                      << " got1: " << static_cast<int> (got1)
                      << " got2: " << static_cast<int> (got2)
                      << " got3: " << static_cast<int> (got3)
                      << std::endl;

            return false;
        }

        curVal = curVal ? curVal * 2 : startVal;
    }

    return true;
}

template <
    typename T,
    unsigned int MAX_ITERATIONS,
    int startVal = 1
>
bool testNonConstExpr ()
{
    return run_test<
        T, MAX_ITERATIONS, startVal, FirstBitSet
    > (FirstBitSet ());
}

template <
    typename T,
    unsigned int MAX_ITERATIONS,
    int startVal = 1
>
bool testConstExpr ()
{
    auto fbs =
        static_cast<unsigned int (*) (T)> (
            &FirstBitSet::findConstExpr
        );

    return run_test<T, MAX_ITERATIONS, startVal, decltype (fbs)> (fbs);
}

std::vector<Test> NumberFirstBitSetTestVec = {
    {testNonConstExpr<int32_t, 32>,       TestName ("testNonCE<int32_t>")},
    {testNonConstExpr<int32_t, 33, -1>,   TestName ("testNonCE<int32_t, -1>")},
    {testNonConstExpr<uint32_t, 33>,      TestName ("testNonCE<uint32_t>")},
    {testNonConstExpr<int64_t, 64>,       TestName ("testNonCE<int64_t>")},
    {testNonConstExpr<int64_t, 65, -1>,   TestName ("testNonCE<int64_t, -1>")},
    {testNonConstExpr<uint64_t, 65>,      TestName ("testNonCE<uint64_t>")},
    {testNonConstExpr<__int128_t, 128>,   TestName ("testNonCE<int128_t>")},
    {
        testNonConstExpr<__int128_t, 129, -1>,
        TestName ("testNonCE<int128_t, -1>")
    },
    {testNonConstExpr<__uint128_t, 129>, TestName ("testNonCE<uint128_t>")},
    {testConstExpr<int64_t, 64>,         TestName ("testConstExpr<int64_t>")},
    {testConstExpr<int64_t, 65, -1>, TestName ("testConstExpr<int64_t, -1>")}
};

} // namespace test
} // namespace fixed
