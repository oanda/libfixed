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

#include <iostream>
#include <vector>
#include <unordered_map>

namespace fixed {
namespace test {

template <typename T>
class ConstructorFailTest {
  public:
    constexpr ConstructorFailTest (
        T intVal,
        uint64_t fracVal,
        uint8_t dp,
        Number::Sign sign
    )
      : intVal_ (intVal),
        fracVal_ (fracVal),
        dp_ (dp),
        sign_ (sign),
        strVal_ (""),
        useStringTest_ (false)
    {
    }

    constexpr ConstructorFailTest (
        const std::string& strVal
    )
      : intVal_ (0),
        fracVal_ (0),
        dp_ (0),
        sign_ (Number::Sign::POSITIVE),
        strVal_ (strVal),
        useStringTest_ (true)
    {
    }

    bool operator() ()
    {
        try {
            if (useStringTest_)
            {
                Number n (strVal_);

                std::cerr << "Error, string constructor expected exception "
                          << "for " << n.toString ()
                          << std::endl;
            }
            else
            {
                Number n (intVal_, fracVal_, dp_, sign_);

                std::cerr << "Error, value constructor expected exception for "
                          << n.toString ()
                          << std::endl;
            }

            return false;
        }
        catch (const fixed::BadValueException)
        {
        }

        return true;
    }

  private:
    T intVal_;
    uint64_t fracVal_;
    unsigned int dp_;
    Number::Sign sign_;
    std::string strVal_;
    bool useStringTest_;
};

template <typename T>
constexpr Test createTest (
    const T intVal,
    const uint64_t fracVal,
    const uint8_t dp,
    const Number::Sign sign,
    const std::string& testName
)
{
    return Test (
        ConstructorFailTest<T> (intVal, fracVal, dp, sign),
        TestName (testName)
    );
}

Test createTest (const std::string& strVal)
{
    return Test (
        ConstructorFailTest<int> (strVal),
        TestName ("'" + strVal + "'")
    );
}

static const Number::Sign NEG = Number::Sign::NEGATIVE;
static const Number::Sign POS = Number::Sign::POSITIVE;

std::vector<Test> NumberIntConstructorFailTestVec = {
    createTest (9223372036854775808ULL, 0, 0, POS, "2^63"),
    createTest ("9223372036854775808"),

    createTest (-9223372036854775807 - 1, 0, 0, POS, "-2^63"),
    createTest (9223372036854775808ULL, 0, 0, NEG, "-2^63"),
    createTest ("-9223372036854775808"),

    createTest (18446744073709551615ULL, 0, 0, POS, "2^64-1"),
    createTest (18446744073709551615ULL, 0, 0, NEG, "-2^64-1"),
    createTest ("18446744073709551615"),
    createTest ("118446744073709551615"),
    createTest (18446744073709551615ULL, 999999999999999999, 18, POS, "BIGP"),
    createTest (18446744073709551615ULL, 999999999999999999, 18, NEG, "BIGN"),
    createTest ("18446744073709551615.999999999999999999"),
    createTest ("-18446744073709551615.999999999999999999"),

    createTest (0, 123456789012345, 14, POS, "Fraction value too large 1"),
    createTest (0, 123456789012345, 15, POS, "Fraction value too large 2"),
    createTest ("0.123456789012345"),
    createTest (".123456789012345"),
    createTest ("0.-1234"),
    createTest ("0.ab324"),
    createTest ("ewr"),
    createTest ("+ewr"),
    createTest ("-ewr"),
    createTest ("-11234K435"),
    createTest ("-11234435B"),
    createTest ("-11234435.0B"),
    createTest (""),
    createTest ("."),
    createTest ("1.")
};

} // namespace test
} // namespace fixed
