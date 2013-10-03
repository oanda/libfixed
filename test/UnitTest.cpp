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

extern std::vector<Test> NumberArithmeticTestVec;
extern std::vector<Test> NumberIntConstructorFailTestVec;
extern std::vector<Test> NumberIntConstructorTestVec;
extern std::vector<Test> NumberFirstBitSetTestVec;
extern std::vector<Test> NumberFpConstructorFailTestVec;
extern std::vector<Test> NumberFpConstructorTestVec;
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
    { "Relational", NumberRelationalTestVec }
  }
};

} // namespace test
} // namespace fixed

int main (int argc, char* argv[], char* envp[])
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
