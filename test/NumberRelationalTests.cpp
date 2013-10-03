#include "fixed/Number.h"
#include "TestsCommon.h"

#include <functional>
#include <iostream>
#include <vector>
#include <unordered_map>

namespace fixed {
namespace test {

struct RelationalOps {
    typedef std::function<
        bool (const std::string& op1, const std::string& op2)
    > OpFunc;

    std::string name;
    std::string mirrorOpName;
    std::string complementOpName;
    OpFunc opFunc;
};

static std::unordered_map<
    std::string, RelationalOps
> relOpsTable = {
    {
        "<",
        {
            "<",
            ">",
            ">=",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) < Number (op2);
            }
        }
    },
    {
        "<=",
        {
            "<=",
            ">=",
            ">",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) <= Number (op2);
            }
        }
    },
    {
        ">",
        {
            ">",
            "<",
            "<=",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) > Number (op2);
            }
        }
    },
    {
        ">=",
        {
            ">=",
            "<=",
            "<",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) >= Number (op2);
            }
        }
    },
    {
        "==",
        {
            "==",
            "==",
            "!=",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) == Number (op2);
            }
        }
    },
    {
        "!=",
        {
            "!=",
            "!=",
            "==",
            [] (const std::string& op1, const std::string& op2)
            {
                return Number (op1) != Number (op2);
            }
        }
    }
};

class RelationalTest {
  public:
    RelationalTest (
        const std::string& op1,
        const std::string& relationalOp,
        const std::string& op2,
        const bool result
    )
      : op1_ (op1),
        relOp_ (relOpsTable[relationalOp]),
        op2_ (op2),
        expectedResult_ (result)
    {
    }

    bool operator() ()
    {
        bool res = relOp_.opFunc (op1_, op2_);

        if (res != expectedResult_)
        {
            std::cerr << op1_ << " " << relOp_.name << " " << op2_
                      << " produced " << (res ? "true" : "false")
                      << " expected " << (expectedResult_ ? "true" : "false")
                      << std::endl;

            return false;
        }

        return true;
    }

  private:
    const std::string op1_;
    const RelationalOps& relOp_;
    const std::string op2_;
    const bool expectedResult_;
};

static Test createTest (
    const std::string& op1,
    const std::string& relationalOp,
    const std::string& op2,
    const bool result
)
{
    return Test (
        RelationalTest (op1, relationalOp, op2, result),
        [=] () {
            return "Base relational " + op1 + " " + relationalOp + " " + op2;
        }
    );
}

#define CREATE_TEST(op1, relOp, op2, res) \
    createTest (op1, relOp, op2, res), \
    createTest (op1, relOpsTable[relOp].complementOpName, op2, !res), \
    createTest (op2, relOpsTable[relOp].mirrorOpName, op1, res), \
    createTest ( \
        op2, \
        relOpsTable[relOpsTable[relOp].mirrorOpName].complementOpName, \
        op1, \
        !res \
    )

std::vector<Test> NumberRelationalTestVec = {
  {
    CREATE_TEST ("1", "<",  "1", false),
    CREATE_TEST ("1", ">",  "1", false),
    CREATE_TEST ("1", "==", "1", true),

    CREATE_TEST ("1", "<",  "1.00000000000000", false),
    CREATE_TEST ("1", ">",  "1.00000000000000", false),
    CREATE_TEST ("1", "==", "1.00000000000000", true),

    CREATE_TEST (
        "9223372036854775807",
        "<",
        "9223372036854775807.00000000000001",
        true
    ),
    CREATE_TEST (
        "9223372036854775807",
        ">",
        "9223372036854775807.00000000000001",
        false
    ),
    CREATE_TEST (
        "9223372036854775807",
        "==",
        "9223372036854775807.00000000000001",
        false
    ),

    CREATE_TEST (
        "9223372036854775807",
        "<",
        "9223372036854775806.99999999999999",
        false
    ),
    CREATE_TEST (
        "9223372036854775807",
        ">",
        "9223372036854775806.99999999999999",
        true
    ),
    CREATE_TEST (
        "9223372036854775807",
        "==",
        "9223372036854775806.99999999999999",
        false
    ),

    //
    // Both operands in 64bit storage internally, different decimal places
    //
    CREATE_TEST ("123.123", "<",  "24.65476", false),
    CREATE_TEST ("123.123", ">",  "24.65476", true),
    CREATE_TEST ("123.123", "==", "24.65476", false),

    //
    // One operand in 64bit storage, other in 128, different decimal places
    //
    CREATE_TEST ("1234567890.123", "<",  "123456789.012345678901", false),
    CREATE_TEST ("1234567890.123", ">",  "123456789.012345678901", true),
    CREATE_TEST ("1234567890.123", "==", "123456789.012345678901", false),

    //
    // Both operands in 128bit storage internally, different decimal places
    //
    CREATE_TEST (
        "1234567890.1234567890", "<", "123456789.012345678901", false
    ),
    CREATE_TEST (
        "1234567890.1234567890", ">", "123456789.012345678901", true
    ),
    CREATE_TEST (
        "1234567890.1234567890", "==", "123456789.012345678901", false
    ),

    //
    // Both operands in 64bit storage internally, same decimal places
    //
    CREATE_TEST ("123.123", "<",  "24.654", false),
    CREATE_TEST ("123.123", ">",  "24.654", true),
    CREATE_TEST ("123.123", "==", "24.654", false),

    //
    // One operand in 64bit storage, other in 128, same decimal places
    //
    CREATE_TEST ("1.123456789012", "<",  "123456789.012345678901", true),
    CREATE_TEST ("1.123456789012", ">",  "123456789.012345678901", false),
    CREATE_TEST ("1.123456789012", "==", "123456789.012345678901", false),

    //
    // Both operands in 128bit storage internally, same decimal places
    //
    CREATE_TEST (
        "1234567890.123456789012", "<", "123456789.012345678901", false
    ),
    CREATE_TEST (
        "1234567890.123456789012", ">", "123456789.012345678901", true
    ),
    CREATE_TEST (
        "1234567890.123456789012", "==", "123456789.012345678901", false
    ),

    //
    // Equivalent values with different decimal places, both 64bit storage
    //
    CREATE_TEST ("123.123", "<",  "123.1230000", false),
    CREATE_TEST ("123.123", ">",  "123.1230000", false),
    CREATE_TEST ("123.123", "==", "123.1230000", true),

    //
    // Equivalent values with different decimal places, one 64bit storage,
    // other 128bit
    //
    CREATE_TEST ("1234567890.123", "<",  "1234567890.12300000000000", false),
    CREATE_TEST ("1234567890.123", ">",  "1234567890.12300000000000", false),
    CREATE_TEST ("1234567890.123", "==", "1234567890.12300000000000", true),

    //
    // Equivalent values with different decimal places, both 128bit storage
    //
    CREATE_TEST (
        "1234567890.1230000000", "<",  "1234567890.12300000000000", false
    ),
    CREATE_TEST (
        "1234567890.1230000000", ">",  "1234567890.12300000000000", false
    ),
    CREATE_TEST (
        "1234567890.1230000000", "==", "1234567890.12300000000000", true
    )
  }
};

} // namespace test
} // namespace fixed
