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
