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

#include "fixed/Number.h"
#include "TestsCommon.h"

#include <functional>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace fixed {
namespace test {

template <typename T>
class ToFpTest {
  public:
    ToFpTest (
        const std::string& strVal,
        const T& floatVal,
        const T& epsilon
    )
      : strVal_ (strVal),
        floatVal_ (floatVal),
        epsilon_ (epsilon)
    {
    }

    bool operator() ()
    {
        T diffAbs;

        if (std::is_same<T, double>::value)
        {
            diffAbs = fabs (Number (strVal_).toDouble () - floatVal_);
        }
        else
        {
            diffAbs = fabsl (Number (strVal_).toLongDouble () - floatVal_);
        }

        if (diffAbs > epsilon_)
        {
            std::cerr << "To floating point test on " << strVal_ << " failed "
                      << "epsilon: " << epsilon_
                      << " actual difference: " << diffAbs
                      << std::endl;

            return false;
        }

        return true;
    }

  private:
    std::string strVal_;
    T floatVal_;
    T epsilon_;
};

template <typename T>
constexpr Test createTest (
    const std::string& strVal,
    const T& floatVal,
    const T& epsilon
)
{
    return Test (
        ToFpTest<T> (strVal, floatVal, epsilon),
        [=] () {
            return std::string ("To floating point test: ") + strVal;
        }
    );
}

std::vector<Test> NumberToFpTestVec = {
  {
    createTest ("1.23456", 1.23456, 0.00000001),
    createTest ("1.23456", 1.23456L, 0.00000001L),
    createTest ("-1.23456", -1.23456, 0.00000001),
    createTest ("-1.23456", -1.23456L, 0.00000001L),

    createTest (
        "234092342341.2234233456",
        234092342341.2234233456L,
        0.0000000000001L
    ),

    createTest (
        "-234092342341.2234233456",
        -234092342341.2234233456L,
        0.0000000000001L
    )

  }
};

} // namespace test
} // namespace fixed
