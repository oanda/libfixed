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
    createTest (-18446744073709551615.000L, "-2^64-1")
};

} // namespace test
} // namespace fixed
