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

//
// There are separate tests for rounding, all results for the arithmetic tests
// here will be using this rounding mode.
//
static const Rounding::Mode ARITHMETIC_TEST_ROUNDING_MODE =
    Rounding::Mode::TO_NEAREST_HALF_TO_EVEN;

struct ArithmeticOps {
    enum class Type {
        PLUS = 0,
        MINUS,
        MULT,
        DIV,
        MOD
    };

    using OpFunc =
        std::function<Number (const std::string& op1, const std::string& op2)>;

    std::string name;
    OpFunc binary;
    OpFunc assignment;
};

} // namespace test
} // namespace fixed

namespace std {
    template <>
    struct hash<fixed::test::ArithmeticOps::Type>
    {
        size_t operator() (const fixed::test::ArithmeticOps::Type& t) const
        {
            return hash<int> () (
                static_cast<
                    std::underlying_type<fixed::test::ArithmeticOps::Type>::type
                > (t)
            );
        }
    };
} // namespace std

namespace fixed {
namespace test {

static const ArithmeticOps::Type PLUS  = ArithmeticOps::Type::PLUS;
static const ArithmeticOps::Type MINUS = ArithmeticOps::Type::MINUS;
static const ArithmeticOps::Type MULT  = ArithmeticOps::Type::MULT;
static const ArithmeticOps::Type DIV   = ArithmeticOps::Type::DIV;
static const ArithmeticOps::Type MOD   = ArithmeticOps::Type::MOD;

static std::unordered_map<
    ArithmeticOps::Type, ArithmeticOps
> arithOpsTable = {
    {
        PLUS,
        {
            "PLUS",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) + Number (op2);
            },
            [] (const std::string& op1, const std::string& op2)
            {
                Number number (op1);
                return number += Number (op2);
            }
        }
    },
    {
        MINUS,
        {
            "MINUS",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) - Number (op2);
            },
            [] (const std::string& op1, const std::string& op2)
            {
                Number number (op1);
                return number -= Number (op2);
            }
        }
    },
    {
        MULT,
        {
            "MULT",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) * Number (op2);
            },
            [] (const std::string& op1, const std::string& op2)
            {
                Number number (op1);
                return number *= Number (op2);
            }
        }
    },
    {
        DIV,
        {
            "DIV",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) / Number (op2);
            },
            [] (const std::string& op1, const std::string& op2)
            {
                Number number (op1);
                return number /= Number (op2);
            }
        }
    },
    {
        MOD,
        {
            "MOD",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) % Number (op2);
            },
            [] (const std::string& op1, const std::string& op2)
            {
                Number number (op1);
                return number %= Number (op2);
            }
        }
    }
};

struct Results {

    struct R {
        Precision::Policy policy;
        std::string value;
    };

    const std::string negateStr;
    std::vector<R> results;
};

class ArithmeticTest {
  public:
    ArithmeticTest (
        const ArithmeticOps::Type arithType,
        const std::string& op1,
        const std::string& op2,
        const Results& results
    )
      : arithOps_ (arithOpsTable[arithType]),
        op1_ (op1),
        op2_ (op2),
        expectedResults_ (results),
        overflowExpected_ (results.results[0].value == "OVERFLOW"),
        divByZeroExpected_ (results.results[0].value == "DIV_BY_ZERO")
    {
    }

    bool runOverflowTest (
        const ArithmeticOps::OpFunc& func,
        const std::string& name
    )
    {
        try {
            const std::string result = func (op1_, op2_).toString ();

            std::cerr << "Test " << name << " expected overflow exception "
                      << "got result: " << result << " instead."
                      << std::endl;

            return false;
        }
        catch (fixed::OverflowException& e)
        {
            return true;
        }
    }

    bool runDivByZeroTest (
        const ArithmeticOps::OpFunc& func,
        const std::string& name
    )
    {
        try {
            const std::string result = func (op1_, op2_).toString ();

            std::cerr << "Test " << name << " expected div by zero exception "
                      << "got result: " << result << " instead."
                      << std::endl;

            return false;
        }
        catch (fixed::DivideByZeroException& e)
        {
            return true;
        }
    }

    bool runNonOverflowTest (
        const ArithmeticOps::OpFunc& func,
        const std::string& expectedResult,
        const std::string& name
    )
    {
        Number result = func (op1_, op2_);

        if (! checkNumber (name, result, Number (expectedResult)))
        {
            std::cerr << "Result of operation was " << result.toString ()
                      << std::endl;

            return false;
        }

        return true;
    }

    bool runTest (const std::string& expectedResult, const std::string& name)
    {
        if (overflowExpected_)
        {
            return (
                runOverflowTest (arithOps_.binary, name + " binary")
                &&
                runOverflowTest (
                    arithOps_.assignment, name + " assignment"
                )
            );
        }
        else if (divByZeroExpected_)
        {
            return (
                runDivByZeroTest (arithOps_.binary, name + " binary")
                &&
                runDivByZeroTest (
                    arithOps_.assignment, name + " assignment"
                )
            );
        }
        else
        {
            return (
                runNonOverflowTest (
                    arithOps_.binary, expectedResult, name + " binary"
                )
                &&
                runNonOverflowTest (
                    arithOps_.assignment, expectedResult, name + " assignment"
                )
            );
        }
    }

    bool operator() ()
    {
        Number::setDefaultRoundingMode (ARITHMETIC_TEST_ROUNDING_MODE);

        for (const auto& r: expectedResults_.results)
        {
            std::string name =
                arithOps_.name + " precision policy '" +
                Precision::policyToString (r.policy) + "'";

            std::string value = expectedResults_.negateStr + r.value;

            Number::setDefaultMultPrecisionPolicy (r.policy);
            Number::setDefaultDivPrecisionPolicy (r.policy);

            if (! runTest (value, name))
            {
                return false;
            }
        }

        return true;
    }

  private:
    const ArithmeticOps& arithOps_;
    const std::string op1_;
    const std::string op2_;
    const Results expectedResults_;
    const bool overflowExpected_;
    const bool divByZeroExpected_;
};

static Test createTest (
    const ArithmeticOps::Type type,
    const std::string& op1,
    const std::string& op2,
    const Results& results
)
{
    return Test (
        ArithmeticTest (type, op1, op2, results),
        [=] () {
            return
                arithOpsTable[type].name + " " + op1 + " " + op2;
        }
    );
}

//
// Setup some short forms for convenience.
//
static const Precision::Policy MIN_OP    = Precision::Policy::MIN_OPERAND;
static const Precision::Policy MIN_OP_P1 = Precision::Policy::MIN_OPERAND_PLUS_1;
static const Precision::Policy MIN_OP_P2 = Precision::Policy::MIN_OPERAND_PLUS_2;
static const Precision::Policy MIN_OP_P3 = Precision::Policy::MIN_OPERAND_PLUS_3;
static const Precision::Policy MIN_OP_P4 = Precision::Policy::MIN_OPERAND_PLUS_4;
static const Precision::Policy MIN_OP_P5 = Precision::Policy::MIN_OPERAND_PLUS_5;
static const Precision::Policy MAX_OP    = Precision::Policy::MAX_OPERAND;
static const Precision::Policy MAX_OP_P1 = Precision::Policy::MAX_OPERAND_PLUS_1;
static const Precision::Policy MAX_OP_P2 = Precision::Policy::MAX_OPERAND_PLUS_2;
static const Precision::Policy MAX_OP_P3 = Precision::Policy::MAX_OPERAND_PLUS_3;
static const Precision::Policy MAX_OP_P4 = Precision::Policy::MAX_OPERAND_PLUS_4;
static const Precision::Policy MAX_OP_P5 = Precision::Policy::MAX_OPERAND_PLUS_5;
static const Precision::Policy MAX_P     = Precision::Policy::MAX_PRECISION;

#define CREATE_PLUS_TEST(op1, op2, res) \
    createTest (PLUS, op1, op2, {"", {{MAX_P, res}}}), \
    createTest (PLUS, op2, op1, {"", {{MAX_P, res}}})

#define CREATE_PLUS_TEST_OVERFLOW(op1, op2) \
    createTest (PLUS, op1, op2, {"", {{MAX_P, "OVERFLOW"}}}), \
    createTest (PLUS, op2, op1, {"", {{MAX_P, "OVERFLOW"}}})

#define CREATE_MINUS_TEST(op1, op2, res) \
    createTest (MINUS, op1, op2, {"", {{MAX_P, res}}})

#define CREATE_MINUS_TEST_OVERFLOW(op1, op2) \
    createTest (MINUS, op1, op2, {"", {{MAX_P, "OVERFLOW"}}})

#define CREATE_MULT_TEST(op1, op2, res...) \
    createTest (MULT, op1, op2, {"", {res}}), \
    createTest (MULT, "-" op1, op2, {"-", {res}}), \
    createTest (MULT, op1, "-" op2, {"-", {res}}), \
    createTest (MULT, "-" op1, "-" op2, {"", {res}}), \
    createTest (MULT, op2, op1, {"", {res}}), \
    createTest (MULT, "-" op2, op1, {"-", {res}}), \
    createTest (MULT, op2, "-" op1, {"-", {res}}), \
    createTest (MULT, "-" op2, "-" op1, {"", {res}})

#define CREATE_MULT_TEST_OVERFLOW(op1, op2) \
    CREATE_MULT_TEST (op1, op2, {MAX_P, "OVERFLOW"})

#define CREATE_DIV_TEST(op1, op2, res...) \
    createTest (DIV, op1, op2, {"", {res}}), \
    createTest (DIV, "-" op1, op2, {"-", {res}}), \
    createTest (DIV, op1, "-" op2, {"-", {res}}), \
    createTest (DIV, "-" op1, "-" op2, {"", {res}})

#define CREATE_DIV_TEST_OVERFLOW(op1, op2) \
    CREATE_DIV_TEST (op1, op2, {MAX_P, "OVERFLOW"})

#define CREATE_DIV_TEST_DIV_BY_ZERO(op1, op2) \
    CREATE_DIV_TEST (op1, op2, {MAX_P, "DIV_BY_ZERO"})

#define CREATE_MOD_TEST(op1, op2, res...) \
    createTest (MOD, op1, op2, {"", {res}}), \
    createTest (MOD, "-" op1, op2, {"-", {res}}), \
    createTest (MOD, op1, "-" op2, {"", {res}}), \
    createTest (MOD, "-" op1, "-" op2, {"-", {res}})

#define CREATE_MOD_TEST_DIV_BY_ZERO(op1, op2) \
    CREATE_MOD_TEST (op1, op2, {MAX_P, "DIV_BY_ZERO"})

std::vector<Test> NumberArithmeticTestVec = {
    CREATE_PLUS_TEST ("0", "0", "0"),
    CREATE_PLUS_TEST ("0", "1", "1"),
    CREATE_PLUS_TEST ("0", "2", "2"),
    CREATE_PLUS_TEST ("0.0", "1", "1.0"),

    //
    // Some random decimal places precision tests
    //
    CREATE_PLUS_TEST ("0.001", "1.0000001", "1.0010001"),
    CREATE_PLUS_TEST ("-0.001", "1.0000001", "0.9990001"),
    CREATE_PLUS_TEST ("0.001", "-1.0000001", "-0.9990001"),
    CREATE_PLUS_TEST ("-0.001", "-1.0000001", "-1.0010001"),
    CREATE_PLUS_TEST ("1.0000001", "0.001", "1.0010001"),
    CREATE_PLUS_TEST ("-1.0000001", "0.001", "-0.9990001"),
    CREATE_PLUS_TEST ("1.0000001", "-0.001", "0.9990001"),
    CREATE_PLUS_TEST ("-1.0000001", "-0.001", "-1.0010001"),

    //
    // Tests around 2^63-1, 2^63 and 2^64-1 for the integer values, which
    // are some internal limit tests inside FixedNumber
    //
    CREATE_PLUS_TEST ("9223372036854775807", "0", "9223372036854775807"),
    CREATE_PLUS_TEST ("-9223372036854775807", "0", "-9223372036854775807"),
    CREATE_PLUS_TEST ("-9223372036854775807", "1", "-9223372036854775806"),
    CREATE_PLUS_TEST ("9223372036854775807", "-1", "9223372036854775806"),

    //
    // Tests around 2^63-1, 2^63 and 2^64-1 for the internal storage of
    // the fixed number as an int64_t
    //
    CREATE_PLUS_TEST (
        "9222136.802854775807", "1235.234", "9223372.036854775807"
    ),
    CREATE_PLUS_TEST (
        "9222136.802854775808", "1235.234", "9223372.036854775808"
    ),
    CREATE_PLUS_TEST (
        "-9222136.802854775807", "-1235.234", "-9223372.036854775807"
    ),
    CREATE_PLUS_TEST (
        "-9222136.802854775808", "-1235.234", "-9223372.036854775808"
    ),
    CREATE_PLUS_TEST (
        "161063205.61363039615", "23404235.12346512", "184467440.73709551615"
    ),
    CREATE_PLUS_TEST (
        "161063205.61363039616", "23404235.12346512", "184467440.73709551616"
    ),
    CREATE_PLUS_TEST (
        "-161063205.61363039615", "-23404235.12346512", "-184467440.73709551615"
    ),
    CREATE_PLUS_TEST (
        "-161063205.61363039616", "-23404235.12346512", "-184467440.73709551616"
    ),

    //
    // Tests where both operands are stored internally as 64 bit and result
    // fits in a 64 bit
    //
    CREATE_PLUS_TEST ("100.200", "200.100", "300.300"),
    CREATE_PLUS_TEST ("-100.200", "-200.100", "-300.300"),
    CREATE_PLUS_TEST ("100.200", "-200.100", "-99.900"),
    CREATE_PLUS_TEST ("-100.200", "200.100", "99.900"),

    //
    // Tests where the two operands fit in a 64 bit, but result must be in128
    //
    CREATE_PLUS_TEST (
        "9223372.036854770807", "12234042.35123465", "21457414.388089420807"
    ),

    CREATE_PLUS_TEST (
        "-9223372.036854770807", "-12234042.35123465", "-21457414.388089420807"
    ),

    //
    // Tests where 1 operand is stored internally as 64 bit, and the other
    // is stored as 128bit, and result is 128 bit
    //
    CREATE_PLUS_TEST (
        "10.20", "1384467440.73709551616", "1384467450.93709551616"
    ),
    CREATE_PLUS_TEST (
        "-10.20", "-1384467440.73709551616", "-1384467450.93709551616"
    ),

    CREATE_PLUS_TEST (
        "9223372036854775807.99999999999998",
        "0.00000000000001",
        "9223372036854775807.99999999999999"
    ),

    CREATE_PLUS_TEST (
        "-9223372036854775807.99999999999998",
        "-0.00000000000001",
        "-9223372036854775807.99999999999999"
    ),

    CREATE_PLUS_TEST (
        "7988804146731319018.87654321098765",
        "1234567890123456789.12345678901234",
        "9223372036854775807.99999999999999"
    ),

    CREATE_PLUS_TEST (
        "-7988804146731319018.87654321098765",
        "-1234567890123456789.12345678901234",
        "-9223372036854775807.99999999999999"
    ),

    CREATE_PLUS_TEST (
        "9223372036854775806.99999999999999",
        "1",
        "9223372036854775807.99999999999999"
    ),

    CREATE_PLUS_TEST_OVERFLOW ("9223372036854775807", "1"),
    CREATE_PLUS_TEST_OVERFLOW ("-9223372036854775807", "-1"),

    CREATE_PLUS_TEST_OVERFLOW (
        "9223372036854775806.99999999999999",
        "1239082520348921034.32402340901234"
    ),

    CREATE_PLUS_TEST_OVERFLOW (
        "-9223372036854775806.99999999999999",
        "-1239082520348921034.32402340901234"
    ),

    CREATE_MINUS_TEST ("0", "0", "0"),
    CREATE_MINUS_TEST ("1", "0", "1"),
    CREATE_MINUS_TEST ("1", "1", "0"),
    CREATE_MINUS_TEST ("0", "2", "-2"),
    CREATE_MINUS_TEST ("0.0", "1", "-1.0"),
    CREATE_MINUS_TEST ("0.001", "1.0000001", "-0.9990001"),
    CREATE_MINUS_TEST ("0.001", "1.00000010", "-0.99900010"),
    CREATE_MINUS_TEST ("0.001", "0.001", "0.000"),

    //
    // Tests where the two operands fit in a 64 bit, but result must be in128,
    // important, as the subtraction will need to align the decimal places
    // must ensure the shifting doesn't cause one of the numbers to require
    // 128bit...
    //
    CREATE_MINUS_TEST (
        "9223372.036854770807", "-1223404.35123465", "10446776.388089420807"
    ),

    CREATE_MINUS_TEST (
        "-9223372.036854770807", "1223404.35123465", "-10446776.388089420807"
    ),

    CREATE_MINUS_TEST ("9223372036854775807", "1", "9223372036854775806"),
    CREATE_MINUS_TEST ("-9223372036854775807", "-1", "-9223372036854775806"),

    CREATE_MINUS_TEST_OVERFLOW ("9223372036854775807", "-1"),
    CREATE_MINUS_TEST_OVERFLOW ("-9223372036854775807", "1"),

    //
    // Both factors are 64bit internal, and result is 64 bit
    //
    CREATE_MULT_TEST ("0", "0", {MAX_P, "0"}),
    CREATE_MULT_TEST ("0", "1", {{MAX_P, "0"}}),
    CREATE_MULT_TEST ("0.0", "1", {{MAX_P, "0.0"}}),

    CREATE_MULT_TEST (
        "12345.12345",
        "54321.54321",
        {MIN_OP,    "670606156.92196"},
        {MIN_OP_P2, "670606156.9219593"},
        {MAX_OP,    "670606156.92196"},
        {MAX_OP_P2, "670606156.9219593"},
        {MAX_P,     "670606156.9219592745"}
    ),

    //
    // Both factors are 64bit internal, and result is 128 bit
    //
    CREATE_MULT_TEST (
        "1234567.123456",
        "54321.98543210",
        {MIN_OP,    "67064137295.326434"},
        {MIN_OP_P2, "67064137295.32643421"},
        {MAX_OP,    "67064137295.32643421"},
        {MAX_OP_P2, "67064137295.3264342053"},
        {MAX_P,     "67064137295.32643420533760"}
    ),

    //
    // One factor is 64bit internal, and the other is 128bit internal
    //
    CREATE_MULT_TEST (
        "67064137295.3264342053",
        "123.123",
        {MIN_OP,    "8257137776212.477"},
        {MIN_OP_P2, "8257137776212.47656"},
        {MAX_OP,    "8257137776212.4765586592"},
        {MAX_OP_P2, "8257137776212.476558659152"},
        {MAX_P,     "8257137776212.4765586591519"}
    ),

    //
    // Both factors are 128 bit internal, result is 128bit.
    //
    // Tests reduction in factors precision, based on the reduction rules
    // to preserve as much precision as possible for the result, the first
    // number will lose 6 decimal places, while the 2nd number will lose 3
    // The first one loses more as it's a few orders of magnitude larger
    // than the 2nd one, so it losing the lower fractional digits has less
    // impact on the resulting value.
    //
    // Internally we effectively end up doing
    // 123456789012.12345679 * 74709314.17104198834 which gives the answer
    // below of 9223372036854775806.79500247491567
    //
    // Note that the true answer of the 2 full factors is:
    // 9223372036854775806.99899284895878
    //
    // Our answer is 0.20399 off.
    //
    CREATE_MULT_TEST (
        "123456789012.12345678901234",
        "74709314.17104198834225",
        {MIN_OP,    "9223372036854775806.79500247491567"},
        {MIN_OP_P2, "9223372036854775806.79500247491567"},
        {MAX_OP,    "9223372036854775806.79500247491567"},
        {MAX_OP_P2, "9223372036854775806.79500247491567"},
        {MAX_P,     "9223372036854775806.79500247491567"}
    ),

    //
    // Tests that the trailing zeros from the second number are trimmed before
    // reducing the precision on the first one, in this case the first number
    // will lose 4 decimal places, and the zeros will be trimmed from the 2nd
    // one.
    //
    // The true answer of the 2 full factors is:
    // 9223372036854775764.74590680955953
    //
    // Our answer is off the true answer by .00092
    //
    CREATE_MULT_TEST (
        "123456789012.12345678901234",
        "74709314.17104198800000",
        {MIN_OP,    "9223372036854775764.74498489662266"},
        {MIN_OP_P2, "9223372036854775764.74498489662266"},
        {MAX_OP,    "9223372036854775764.74498489662266"},
        {MAX_OP_P2, "9223372036854775764.74498489662266"},
        {MAX_P,     "9223372036854775764.74498489662266"}
    ),

    //
    // In this case only the zeros from the second factor are stripped, no
    // reduction in precision of the first one needs to occur.
    //
    // In this case our answser matches the exact answer.
    //
    CREATE_MULT_TEST (
        "123456789012.12345678901234",
        "74709314.17104000000000",
        {MIN_OP,    "9223372036854530332.64935070812743"},
        {MIN_OP_P2, "9223372036854530332.64935070812743"},
        {MAX_OP,    "9223372036854530332.64935070812743"},
        {MAX_OP_P2, "9223372036854530332.64935070812743"},
        {MAX_P,     "9223372036854530332.64935070812743"}
    ),

    CREATE_MULT_TEST (
        "123456789012.12345000000000",
        "74709314.17104000000000",
        {MIN_OP,    "9223372036854529825.44689488800000"},
        {MIN_OP_P2, "9223372036854529825.44689488800000"},
        {MAX_OP,    "9223372036854529825.44689488800000"},
        {MAX_OP_P2, "9223372036854529825.44689488800000"},
        {MAX_P,     "9223372036854529825.44689488800000"}
    ),

    //
    // Tests one of the corner cases in the precision reduction code, hard
    // to hit else clause, will choose to reduce one digit of precision from
    // the 2nd number here.
    //
    CREATE_MULT_TEST (
        "3037000499.1234567899",
        "3037000499.9876543211",
        {MIN_OP,    "9223372034300693999.3696769711"},
        {MIN_OP_P2, "9223372034300693999.369676971102"},
        {MAX_OP,    "9223372034300693999.3696769711"},
        {MAX_OP_P2, "9223372034300693999.369676971102"},
        {MAX_P,     "9223372034300693999.36967697110152"}
    ),

    CREATE_MULT_TEST (
        "0.99999999999999",
        "0.99999999999999",
        {MIN_OP,    "0.99999999999998"},
        {MIN_OP_P2, "0.99999999999998"},
        {MAX_OP,    "0.99999999999998"},
        {MAX_OP_P2, "0.99999999999998"},
        {MAX_P,     "0.99999999999998"}
    ),

    CREATE_MULT_TEST (
        "123456789012345678.12345678901234",
        "0",
        {MIN_OP,    "0"},
        {MIN_OP_P2, "0.00"},
        {MAX_OP,    "0.00000000000000"},
        {MAX_OP_P2, "0.00000000000000"},
        {MAX_P,     "0.00000000000000"}
    ),

    CREATE_MULT_TEST (
        "123456789012345678.12345678901234",
        "0.0",
        {MIN_OP,    "0.0"},
        {MIN_OP_P2, "0.000"},
        {MAX_OP,    "0.00000000000000"},
        {MAX_OP_P2, "0.00000000000000"},
        {MAX_P,     "0.00000000000000"}
    ),

    CREATE_MULT_TEST (
        "123456789012345678.12345678901234",
        "1",
        {MIN_OP,    "123456789012345678"},
        {MIN_OP_P2, "123456789012345678.12"},
        {MAX_OP,    "123456789012345678.12345678901234"},
        {MAX_OP_P2, "123456789012345678.12345678901234"},
        {MAX_P,     "123456789012345678.12345678901234"}
    ),

    CREATE_MULT_TEST (
        "123456789012345678.12345678901234",
        "1.0",
        {MIN_OP,    "123456789012345678.1"},
        {MIN_OP_P2, "123456789012345678.123"},
        {MAX_OP,    "123456789012345678.12345678901234"},
        {MAX_OP_P2, "123456789012345678.12345678901234"},
        {MAX_P,     "123456789012345678.12345678901234"}
    ),

    CREATE_MULT_TEST (
        "922337203685477580.7",
        "10",
        {MIN_OP,    "9223372036854775807"},
        {MIN_OP_P2, "9223372036854775807.0"},
        {MAX_OP,    "9223372036854775807.0"},
        {MAX_OP_P2, "9223372036854775807.0"},
        {MAX_P,     "9223372036854775807.0"}
    ),

    CREATE_MULT_TEST_OVERFLOW (
        "922337203685477580.8",
        "10"
    ),

    CREATE_MULT_TEST_OVERFLOW (
        "67064137295.3264342053",
        "12234902340980023.123"
    ),

    CREATE_MULT_TEST_OVERFLOW (
        "9223372036854775807.99999999999999",
        "9223372036854775807.99999999999999"
    ),

    CREATE_DIV_TEST (
        "12345.12345",
        "20.12",
        {MIN_OP,    "613.57"},
        {MIN_OP_P2, "613.5747"},
        {MAX_OP,    "613.57472"},
        {MAX_OP_P2, "613.5747242"},
        {MAX_P,     "613.57472415506958"}
    ),

    //
    // Tests revolved around the max representable value
    //
    CREATE_DIV_TEST (
        "9223372036854775807",
        "1",
        {MIN_OP,    "9223372036854775807"},
        {MIN_OP_P2, "9223372036854775807.00"},
        {MAX_OP,    "9223372036854775807"},
        {MAX_OP_P2, "9223372036854775807.00"},
        {MAX_P,      "9223372036854775807.00000000000000"}
    ),

    CREATE_DIV_TEST (
        "922337203685477580.7",
        "0.1",
        {MIN_OP,    "9223372036854775807.0"},
        {MIN_OP_P2, "9223372036854775807.000"},
        {MAX_OP,    "9223372036854775807.0"},
        {MAX_OP_P2, "9223372036854775807.000"},
        {MAX_P,     "9223372036854775807.00000000000000"}
    ),

    CREATE_DIV_TEST (
        "92233.72036854775807",
        "0.00000000000001",
        {MIN_OP,    "9223372036854775807.00000000000000"},
        {MIN_OP_P2, "9223372036854775807.00000000000000"},
        {MAX_OP,    "9223372036854775807.00000000000000"},
        {MAX_OP_P2, "9223372036854775807.00000000000000"},
        {MAX_P,     "9223372036854775807.00000000000000"}
    ),

    CREATE_DIV_TEST (
        "9223372036854775807.99999999999999",
        "31",
        {MIN_OP,    "297528130221121800"},
        {MIN_OP_P1, "297528130221121800.3"},
        {MIN_OP_P2, "297528130221121800.26"},
        {MIN_OP_P3, "297528130221121800.258"},
        {MIN_OP_P4, "297528130221121800.2581"},
        {MIN_OP_P5, "297528130221121800.25806"},
        {MAX_OP,    "297528130221121800.25806451612903"},
        {MAX_OP_P1, "297528130221121800.25806451612903"},
        {MAX_OP_P2, "297528130221121800.25806451612903"},
        {MAX_OP_P3, "297528130221121800.25806451612903"},
        {MAX_OP_P4, "297528130221121800.25806451612903"},
        {MAX_OP_P5, "297528130221121800.25806451612903"},
        {MAX_P,     "297528130221121800.25806451612903"}
    ),

    CREATE_DIV_TEST (
        "9223372036854775807.99999999999999",
        "1000000000000000000.00000000000000",
        {MIN_OP,    "9.22337203685478"},
        {MIN_OP_P2, "9.22337203685478"},
        {MAX_OP,    "9.22337203685478"},
        {MAX_OP_P2, "9.22337203685478"},
        {MAX_P,     "9.22337203685478"}
    ),

    CREATE_DIV_TEST (
        "9223372036854775807.99999999999999",
        "1000000000000000000",
        {MIN_OP,    "9"},
        {MIN_OP_P1, "9.2"},
        {MIN_OP_P2, "9.22"},
        {MIN_OP_P3, "9.223"},
        {MIN_OP_P4, "9.2234"},
        {MIN_OP_P5, "9.22337"},
        {MAX_OP,    "9.22337203685478"},
        {MAX_OP_P1, "9.22337203685478"},
        {MAX_OP_P2, "9.22337203685478"},
        {MAX_OP_P3, "9.22337203685478"},
        {MAX_OP_P4, "9.22337203685478"},
        {MAX_OP_P5, "9.22337203685478"},
        {MAX_P,     "9.22337203685478"}
    ),

    CREATE_DIV_TEST (
        "9223372036854775807",
        "1.12345678901234",
        {MIN_OP,    "8209814678287076241"},
        {MIN_OP_P2, "8209814678287076240.96"},
        {MAX_OP,    "8209814678287076240.96251"},
        {MAX_OP_P2, "8209814678287076240.96251"},
        {MAX_P,     "8209814678287076240.96251"}
    ),

    CREATE_DIV_TEST (
        "922337203685477580",
        "1.1234567899",
        {MIN_OP,    "820981467180037896"},
        {MIN_OP_P2, "820981467180037896.00"},
        {MAX_OP,    "820981467180037895.9995459990"},
        {MAX_OP_P2, "820981467180037895.9995459990"},
        {MAX_P,     "820981467180037895.9995459990"}
    ),

    //
    // On division we will sometimes have to provide less precision in the
    // answer than we'd normally do, here is an extreme case of a very
    // large number divided by a very small number with 14 decimal places.
    // We're only able to provide 4 decimal places of precision in this case,
    // however the result is accurate upto those 4 decimal places.
    //
    CREATE_DIV_TEST (
        "3676299675362152112.41203440812031",
        "0.39858520947355",
        {MIN_OP,    "9223372036854544405.23297"},
        {MIN_OP_P2, "9223372036854544405.23297"},
        {MAX_OP,    "9223372036854544405.23297"},
        {MAX_OP_P2, "9223372036854544405.23297"},
        {MAX_P,     "9223372036854544405.23297"}
    ),

    CREATE_DIV_TEST (
        "3676299675362152112.41203440812031",
        "0.39858520947354",
        {MIN_OP, "9223372036854775807.99999"},
        {MAX_OP, "9223372036854775807.99999"},
        {MAX_P,  "9223372036854775807.99999"}
    ),

    CREATE_DIV_TEST (
        "922337203685477580.7",
        "0.1",
        {MIN_OP,    "9223372036854775807.0"},
        {MIN_OP_P2, "9223372036854775807.000"},
        {MAX_OP,    "9223372036854775807.0"},
        {MAX_OP_P2, "9223372036854775807.000"},
        {MAX_P,     "9223372036854775807.00000000000000"}
    ),

    CREATE_DIV_TEST_OVERFLOW ("922337203685477580.8", "0.1"),

    CREATE_DIV_TEST_OVERFLOW ("1844674407370955161.6", "0.1"),

    CREATE_DIV_TEST_DIV_BY_ZERO ("1.0", "0"),
    CREATE_DIV_TEST_DIV_BY_ZERO ("1.0", "0.0"),
    CREATE_DIV_TEST_DIV_BY_ZERO ("1.0", "0.00"),
    CREATE_DIV_TEST_DIV_BY_ZERO ("1.0", "0.00000000000000"),

    CREATE_MOD_TEST (
        "1",
        "2",
        {MIN_OP,    "1"},
        {MIN_OP_P2, "1"},
        {MAX_OP,    "1"},
        {MAX_OP_P2, "1"},
        {MAX_P,     "1"}
    ),

    CREATE_MOD_TEST (
        "9.2345",
        "2.41",
        {MIN_OP,    "2.0045"},
        {MIN_OP_P2, "2.0045"},
        {MAX_OP,    "2.0045"},
        {MAX_OP_P2, "2.0045"},
        {MAX_P,     "2.0045"}
    ),

    CREATE_MOD_TEST (
        "2.41",
        "9.2345",
        {MIN_OP,    "2.4100"},
        {MIN_OP_P2, "2.4100"},
        {MAX_OP,    "2.4100"},
        {MAX_OP_P2, "2.4100"},
        {MAX_P,     "2.4100"}
    ),

    CREATE_MOD_TEST (
        "9223372036854775807.99999999999999",
        "0.1",
        {MIN_OP,    "0.09999999999999"},
        {MIN_OP_P2, "0.09999999999999"},
        {MAX_OP,    "0.09999999999999"},
        {MAX_OP_P2, "0.09999999999999"},
        {MAX_P,     "0.09999999999999"}
    ),

    CREATE_MOD_TEST (
        "0.1",
        "9223372036854775807.99999999999999",
        {MIN_OP,    "0.10000000000000"},
        {MIN_OP_P2, "0.10000000000000"},
        {MAX_OP,    "0.10000000000000"},
        {MAX_OP_P2, "0.10000000000000"},
        {MAX_P,     "0.10000000000000"}
    ),

    CREATE_MOD_TEST (
        "223372036854775807.99999999999999",
        "123456789012345678.123456789",
        {MIN_OP,    "99915247842430129.87654321099999"},
        {MIN_OP_P2, "99915247842430129.87654321099999"},
        {MAX_OP,    "99915247842430129.87654321099999"},
        {MAX_OP_P2, "99915247842430129.87654321099999"},
        {MAX_P,     "99915247842430129.87654321099999"}
    ),

    CREATE_MOD_TEST (
        "123456789012345678.123456789",
        "223372036854775807.99999999999999",
        {MIN_OP,    "123456789012345678.12345678900000"},
        {MIN_OP_P2, "123456789012345678.12345678900000"},
        {MAX_OP,    "123456789012345678.12345678900000"},
        {MAX_OP_P2, "123456789012345678.12345678900000"},
        {MAX_P,     "123456789012345678.12345678900000"}
    ),

    CREATE_MOD_TEST (
        "0",
        "223372036854775807.99999999999999",
        {MIN_OP,    "0.00000000000000"},
        {MIN_OP_P2, "0.00000000000000"},
        {MAX_OP,    "0.00000000000000"},
        {MAX_OP_P2, "0.00000000000000"},
        {MAX_P,     "0.00000000000000"}
    ),

    CREATE_MOD_TEST (
        "0",
        "1",
        {MIN_OP,    "0"},
        {MIN_OP_P2, "0"},
        {MAX_OP,    "0"},
        {MAX_OP_P2, "0"},
        {MAX_P,     "0"}
    ),

    CREATE_MOD_TEST_DIV_BY_ZERO ("1.0", "0"),
    CREATE_MOD_TEST_DIV_BY_ZERO ("1.0", "0.0"),
    CREATE_MOD_TEST_DIV_BY_ZERO ("1.0", "0.00"),
    CREATE_MOD_TEST_DIV_BY_ZERO ("1.0", "0.00000000000000"),
    CREATE_MOD_TEST_DIV_BY_ZERO ("123456789012345678.12345678901234", "0"),
    CREATE_MOD_TEST_DIV_BY_ZERO ("123456789012345678.12345678901234", "0.0"),
    CREATE_MOD_TEST_DIV_BY_ZERO ("123456789012345678.12345678901234", "0.00"),

    CREATE_MOD_TEST_DIV_BY_ZERO (
        "123456789012345678.12345678901234",
        "0.00000000000000"
    )
};

} // namespace test
} // namespace fixed
