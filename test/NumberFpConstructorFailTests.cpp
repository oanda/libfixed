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

#include <cmath>
#include <iostream>
#include <vector>

namespace fixed {
namespace test {

template <typename T>
class ConstructorFailTest {
  public:
    constexpr ConstructorFailTest (const T floatVal)
      : floatVal_ (floatVal)
    {
    }

    bool operator() ()
    {
        try {
            Number n (floatVal_);

            std::cerr << "Error, value constructor expected exception for "
                      << n.toString ()
                      << std::endl;

            return false;
        }
        catch (const fixed::BadValueException)
        {
        }

        return true;
    }

  private:
    T floatVal_;
};

template <typename T>
constexpr Test createTest (
    const T floatVal,
    const std::string& testName
)
{
    return Test (
        ConstructorFailTest<T> (floatVal),
        TestName (testName)
    );
}

std::vector<Test> NumberFpConstructorFailTestVec = {
    createTest (9300000000000000000.0000000000f, "~ 2^63"),
    createTest (9300000000000000000.0000000000, "~ 2^63"),
    createTest (9223372036854775808.0000000000L, "2^63"),

    createTest (-9300000000000000000.0000000000f, "~ -2^63"),
    createTest (-9300000000000000000.0000000000, "~ -2^63"),
    createTest (-9223372036854775808.0000000000L, "-2^63"),

    createTest (18446744073709551615.000f, "2^64-1"),
    createTest (18446744073709551615.000, "2^64-1"),
    createTest (18446744073709551615.000L, "2^64-1"),

    createTest (-18446744073709551615.000f, "-2^64-1"),
    createTest (-18446744073709551615.000, "-2^64-1"),
    createTest (-18446744073709551615.000L, "-2^64-1"),

    createTest (static_cast<float> (std::nan ("")), "float nan"),
    createTest (static_cast<double> (std::nan ("")), "double nan"),
    createTest (static_cast<long double> (std::nan ("")), "long double nan"),

    createTest (static_cast<float> (INFINITY),  "float +inf"),
    createTest (static_cast<float> (-INFINITY), "float -inf"),

    createTest (static_cast<double> (INFINITY),  "double +inf"),
    createTest (static_cast<double> (-INFINITY), "double -inf"),

    createTest (static_cast<long double> (INFINITY),  "long double +inf"),
    createTest (static_cast<long double> (-INFINITY), "long double -inf")
};

} // namespace test
} // namespace fixed
