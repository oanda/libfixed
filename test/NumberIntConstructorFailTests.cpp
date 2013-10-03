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
        bool negativeFlag
    )
      : intVal_ (intVal),
        fracVal_ (fracVal),
        dp_ (dp),
        negativeFlag_ (negativeFlag),
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
        negativeFlag_ (false),
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
                Number n (intVal_, fracVal_, dp_, negativeFlag_);

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
    bool negativeFlag_;
    std::string strVal_;
    bool useStringTest_;
};

template <typename T>
constexpr Test createTest (
    const T intVal,
    const uint64_t fracVal,
    const uint8_t dp,
    const bool negativeFlag,
    const std::string& testName
)
{
    return Test (
        ConstructorFailTest<T> (intVal, fracVal, dp, negativeFlag),
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

static const bool NEG = true;
static const bool POS = ! NEG;

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
