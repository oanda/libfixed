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
#include <typeinfo>
#include <vector>
#include <unordered_map>

namespace fixed {
namespace test {
 
//
// There are separate tests for rounding, all results for the arithmetic tests
// here will be using this rounding mode.
//
static const Rounding::Mode FP_CONSTRUCTOR_TEST_ROUNDING_MODE =
    Rounding::Mode::TO_NEAREST_HALF_TO_EVEN;

template <typename T>
class ConstructorTest {
  public:
    ConstructorTest (
        const T floatVal,
        unsigned int dp,
        unsigned int expectedDp,
        uint64_t expectedIntVal,
        uint64_t expectedFracVal,
        const std::string& expectedStrVal,
        const bool expectedVal64Set
    )
      : floatVal_ (floatVal),
        dp_ (dp),
        expectedDp_ (expectedDp),
        expectedIntVal_ (expectedIntVal),
        expectedFracVal_ (expectedFracVal),
        expectedStrVal_ (expectedStrVal),
        expectedNegative_ (floatVal < 0.0),
        expectedVal64Set_ (expectedVal64Set)
    {
    }

    bool operator() ()
    {
        Number::setDefaultRoundingMode (FP_CONSTRUCTOR_TEST_ROUNDING_MODE);

        //
        // Note, since we're doing constructor tests, must use the full version
        // of checkNumber here, not the shortened version that takes a
        // reference number object.  The other Number tests can use that
        // version, however here we must ensure the constructors themselves
        // are constructing the number properly, then it's valid for those
        // other tests to use the shortened version.
        //
        std::string name =
            std::string ("Floating Point Constructor ") + typeid (T).name ();

        return
            checkNumber (
                name,
                Number (floatVal_, dp_),
                expectedStrVal_,
                expectedIntVal_,
                expectedFracVal_,
                expectedDp_,
                expectedNegative_,
                expectedVal64Set_
            )
        ;
    }

  private:
    T floatVal_;
    unsigned int dp_;
    unsigned int expectedDp_;
    uint64_t expectedIntVal_;
    uint64_t expectedFracVal_;
    std::string expectedStrVal_;
    bool expectedNegative_;
    uint64_t expectedVal64Set_;
};

static const bool V64 = true;
static const bool V128 = ! V64;

template <typename T>
constexpr Test createTest (
    const T floatVal,
    unsigned int dp,
    unsigned int expectedDp,
    uint64_t expectedIntVal,
    uint64_t expectedFracVal,
    const std::string& expectedStrVal,
    const bool expectedVal64Set
)
{
    return Test (
        ConstructorTest<T> (
            floatVal,
            dp,
            expectedDp,
            expectedIntVal,
            expectedFracVal,
            expectedStrVal,
            expectedVal64Set
        ),
        TestName (expectedStrVal)
    );
}

std::vector<Test> NumberFpConstructorTestVec = {
    createTest (1.2f, 2, 2, 1, 20, "1.20", V64),
    createTest (1.2, 2, 2, 1, 20, "1.20", V64),
    createTest (1.2L, 2, 2, 1, 20, "1.20", V64),
    createTest (1.123456f, 5, 5, 1, 12346, "1.12346", V64),
    createTest (1.123456, 5, 5, 1, 12346, "1.12346", V64),
    createTest (1.123456L, 5, 5, 1, 12346, "1.12346", V64),
    createTest (-1.2f, 2, 2, 1, 20, "-1.20", V64),
    createTest (-1.2, 2, 2, 1, 20, "-1.20", V64),
    createTest (-1.2L, 2, 2, 1, 20, "-1.20", V64),
    createTest (-1.123456f, 5, 5, 1, 12346, "-1.12346", V64),
    createTest (-1.123456, 5, 5, 1, 12346, "-1.12346", V64),
    createTest (-1.123456L, 5, 5, 1, 12346, "-1.12346", V64),

    createTest (
        1234567891012345678.2L,
        1,
        1,
        1234567891012345678,
        2,
        "1234567891012345678.2",
        V128
    ),

    createTest (
        -1234567891012345678.2L,
        1,
        1,
        1234567891012345678,
        2,
        "-1234567891012345678.2",
        V128
    ),

    //
    // By passing in max decimal places + 1 we trigger the decimal place
    // minimization Number will do by trimming excess zeros.
    //
    createTest (
        3.200000, Number::MAX_DECIMAL_PLACES + 1, 1, 3, 2, "3.2", V64
    )

};

} // namespace test
} // namespace fixed
