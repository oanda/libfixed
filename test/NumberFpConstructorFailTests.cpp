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
