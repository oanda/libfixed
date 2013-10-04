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

#ifndef FIXED_TESTS_COMMON_H
#define FIXED_TESTS_COMMON_H

#include "fixed/Number.h"

#include <functional>
#include <iostream>
#include <vector>

namespace fixed {
namespace test {

struct Test {
    Test (
        const std::function<bool ()>& f,
        const std::function<std::string ()> n
    ) : func (f), name (n) {}

    const std::function<bool ()> func;
    const std::function<std::string ()> name;
};

struct TestName {
    TestName (const std::string& n) : name (n) {}

    const std::string& operator() () { return name; }

    std::string name;
};

template <typename VAL>
inline bool valCheck (VAL expectedVal, VAL gotVal, const std::string errMsgHdr)
{
    if (expectedVal != gotVal)
    {
        std::cerr << errMsgHdr
                  << "Expected: " << expectedVal
                  << " got: " << gotVal
                  << std::endl;

        return false;
    }

    return true;
}

inline bool checkNumber (
    const std::string& errMsgHdr,
    const Number& num,
    const std::string& expectedStr,
    const uint64_t expectedInt,
    const uint64_t expectedFrac,
    const unsigned int expectedDp,
    const bool expectedNeg,
    const bool expectedValue64Set
)
{
    return
        valCheck (expectedStr, num.toString (), errMsgHdr + " toString ")
        &&
        valCheck (expectedInt, num.integerValue (), errMsgHdr + " intVal ")
        &&
        valCheck (
            expectedFrac, num.fractionalValue (), errMsgHdr + " fracVal "
        )
        &&
        valCheck (
            expectedDp, num.decimalPlaces (), errMsgHdr + " decimalPlaces "
        )
        &&
        valCheck (
            expectedNeg, num.isNegative (), errMsgHdr + " negative "
        )
        &&
        valCheck (
            expectedValue64Set, num.value64Set (), errMsgHdr + " val64Set "
        )
    ;
}

inline bool checkNumber (
    const std::string& errMsgHdr,
    const Number& num,
    const Number& expectedNum
)
{
    return
        valCheck (
            expectedNum.toString (), num.toString (), errMsgHdr + " toString "
        )
        &&
        valCheck (
            expectedNum.integerValue (),
            num.integerValue (),
            errMsgHdr + " intVal "
        )
        &&
        valCheck (
            expectedNum.fractionalValue (),
            num.fractionalValue (),
            errMsgHdr + " fracVal "
        )
        &&
        valCheck (
            expectedNum.decimalPlaces (),
            num.decimalPlaces (),
            errMsgHdr + " decimalPlaces "
        )
        &&
        valCheck (
            expectedNum.isNegative (),
            num.isNegative (),
            errMsgHdr + " negative "
        )
        &&
        valCheck (
            expectedNum.value64Set (),
            num.value64Set (),
            errMsgHdr + " val64Set "
        )
    ;
}

} // namespace test
} // namespace fixed

#endif // FIXED_TESTS_COMMON_H
