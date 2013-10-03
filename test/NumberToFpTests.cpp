#include "fixed/Number.h"
#include "TestsCommon.h"

#include <functional>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace fixed {
namespace test {

template <typename T>
class ToFpTest {
  public:
    ToFpTest (
        const std::string& strVal,
        const T& floatVal,
        const T& epsilon
    )
      : strVal_ (strVal),
        floatVal_ (floatVal),
        epsilon_ (epsilon)
    {
    }

    bool operator() ()
    {
        T diffAbs;

        if (std::is_same<T, double>::value)
        {
            diffAbs = fabs (Number (strVal_).toDouble () - floatVal_);
        }
        else
        {
            diffAbs = fabsl (Number (strVal_).toLongDouble () - floatVal_);
        }

        if (diffAbs > epsilon_)
        {
            std::cerr << "To floating point test on " << strVal_ << " failed "
                      << "epsilon: " << epsilon_
                      << " actual difference: " << diffAbs
                      << std::endl;

            return false;
        }

        return true;
    }

  private:
    std::string strVal_;
    T floatVal_;
    T epsilon_;
};

template <typename T>
constexpr Test createTest (
    const std::string& strVal,
    const T& floatVal,
    const T& epsilon
)
{
    return Test (
        ToFpTest<T> (strVal, floatVal, epsilon),
        [=] () {
            return std::string ("To floating point test: ") + strVal;
        }
    );
}

std::vector<Test> NumberToFpTestVec = {
  {
    createTest ("1.23456", 1.23456, 0.00000001),
    createTest ("1.23456", 1.23456L, 0.00000001L),
    createTest ("-1.23456", -1.23456, 0.00000001),
    createTest ("-1.23456", -1.23456L, 0.00000001L),

    createTest (
        "234092342341.2234233456",
        234092342341.2234233456L,
        0.0000000000001L
    ),

    createTest (
        "-234092342341.2234233456",
        -234092342341.2234233456L,
        0.0000000000001L
    )

  }
};

} // namespace test
} // namespace fixed
