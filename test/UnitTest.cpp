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

struct TestVec {
    std::string name;
    const std::vector<Test>& tests;
};

extern std::vector<Test> NumberAbsoluteTestVec;
extern std::vector<Test> NumberArithmeticTestVec;
extern std::vector<Test> NumberIntConstructorFailTestVec;
extern std::vector<Test> NumberIntConstructorTestVec;
extern std::vector<Test> NumberFirstBitSetTestVec;
extern std::vector<Test> NumberFpConstructorFailTestVec;
extern std::vector<Test> NumberFpConstructorTestVec;
extern std::vector<Test> NumberNegateTestVec;
extern std::vector<Test> NumberRelationalTestVec;
extern std::vector<Test> NumberRoundingTestVec;
extern std::vector<Test> NumberSqueezeZerosTestVec;
extern std::vector<Test> NumberToFpTestVec;

static std::vector<TestVec> testVecs = {
  {
    { "FirstBitSet", NumberFirstBitSetTestVec },
    { "SqueezeZerosTestVec", NumberSqueezeZerosTestVec },
    { "Integer Constructor", NumberIntConstructorTestVec },
    { "FloatingPoint Constructor", NumberFpConstructorTestVec },
    { "Number To FloatingPoint", NumberToFpTestVec },
    { "Rounding", NumberRoundingTestVec },
    { "Integer Constructor Fail", NumberIntConstructorFailTestVec },
    { "Floating Point Constructor Fail", NumberFpConstructorFailTestVec },
    { "Arithmetic", NumberArithmeticTestVec },
    { "Relational", NumberRelationalTestVec },
    { "Absolute", NumberAbsoluteTestVec },
    { "Negate", NumberNegateTestVec }
  }
};

} // namespace test
} // namespace fixed

int main ()
{
    std::cout << "Beginning Fixed Number library tests." << std::endl;

    for (const auto& tvec: fixed::test::testVecs)
    {
        std::cout << "Running " << tvec.name << " tests." << std::endl;

        for (const auto& t: tvec.tests)
        {
            try {
                if (! t.func ())
                {
                    std::cerr << "Test: " << t.name () << " failed.\n";
                    exit (1);
                }
            }
            catch (std::exception& e)
            {
                std::cerr << "Test: " << t.name ()
                          << " caused unexpected exception: "
                          << e.what () << std::endl;
                exit (1);
            }
        }

        std::cout << "Finished " << tvec.name << " tests.  All passed."
                  << std::endl;
    }

    std::cout << "All Fixed Number library tests passed.\n";

    exit (0);
}
