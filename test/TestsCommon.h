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
