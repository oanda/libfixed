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

#include "TestsCommon.h"

#include <functional>
#include <iostream>
#include <vector>

namespace fixed {
namespace test {

class NegateTest {
  public:
    NegateTest (
        const std::string& input,
        const std::string& expectedResult
    )
      : input_ (input),
        expectedResult_ (expectedResult)
    {}

    bool operator() ()
    {
        Number n (input_);
        Number expectedNumber (expectedResult_);

        if (! checkNumber ("Static", Number::negate (n), expectedNumber))
        {
            return false;
        }

        if (! checkNumber ("Unary -", -n, expectedNumber))
        {
            return false;
        }

        n.negate ();

        if (! checkNumber ("In place", n, expectedNumber))
        {
            return false;
        }

        return true;
    }

  private:
    const std::string input_;
    const std::string expectedResult_;
};

static Test createTest (
    const std::string& input,
    const std::string& expectedResult
)
{
    return Test (
        NegateTest (input, expectedResult),
        [=] () {
            return "Negate '" + input + "'";
        }
    );
}

std::vector<Test> NumberNegateTestVec = {
    createTest ("0.0", "0.0"),
    createTest ("1.00", "-1.00"),
    createTest ("-1.00", "1.00"),
    createTest (
        "123456789012345678.12345678901234",
        "-123456789012345678.12345678901234"
    ),
    createTest (
        "-123456789012345678.12345678901234",
        "123456789012345678.12345678901234"
    )
};

} // namespace test
} // namespace fixed
