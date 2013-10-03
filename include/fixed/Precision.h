#ifndef FIXED_PRECISION_H
#define FIXED_PRECISION_H

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace fixed {

class Precision {
  public:
    //
    // Policy to control how much precision is kept for the results of
    // multiplication.  The policies of the two factors will be compared, and
    // the one that leads to the most precision will be kept.
    //
    enum class Policy : uint8_t {
        MIN_OPERAND = 0,
        MIN_OPERAND_PLUS_1,
        MIN_OPERAND_PLUS_2,
        MIN_OPERAND_PLUS_3,
        MIN_OPERAND_PLUS_4,
        MIN_OPERAND_PLUS_5,
        MAX_OPERAND,
        MAX_OPERAND_PLUS_1,
        MAX_OPERAND_PLUS_2,
        MAX_OPERAND_PLUS_3,
        MAX_OPERAND_PLUS_4,
        MAX_OPERAND_PLUS_5,
        MAX_PRECISION,
        POLICY_MAX_VAL // DO NOT PUT ANY MORE ENUMS AFTER THIS
    };

    static unsigned int getProductDecimalPlaces (
        const unsigned int factor1DecimalPlaces,
        const unsigned int factor2DecimalPlaces,
        const unsigned int maxInternalDecimalPlaces,
        const Policy precisionPolicy
    );

    static unsigned int getQuotientDecimalPlaces (
        const unsigned int dividendDecimalPlaces,
        const unsigned int divisorDecimalPlaces,
        const unsigned int maxInternalDecimalPlaces,
        const Policy precisionPolicy
    );

    static const std::string& policyToString (Policy policy);

  private:
    static const std::vector<
        std::function<
            unsigned int (
                const unsigned int factor1DecimalPlaces,
                const unsigned int factor2DecimalPlaces,
                const unsigned int maxInternalDecimalPlaces
            )
        >
    > productDecimalPlaces_;

    static const std::vector<
        std::function<
            unsigned int (
                const unsigned int dividendDecimalPlaces,
                const unsigned int divisorDecimalPlaces,
                const unsigned int maxInternalDecimalPlaces
            )
        >
    > quotientDecimalPlaces_;

    static const std::vector<std::string> policyStrings_;

    struct RunTimePolicyStringsCheck {
        RunTimePolicyStringsCheck ();
    };

    //
    // This will ensure if someone updates the Policy Enum then the
    // policyStrings_ will be updated as well.
    //
    static const RunTimePolicyStringsCheck runTimePolicyStringsCheck_;
};

inline unsigned int Precision::getProductDecimalPlaces (
    const unsigned int factor1DecimalPlaces,
    const unsigned int factor2DecimalPlaces,
    const unsigned int maxInternalDecimalPlaces,
    const Policy precisionPolicy
)
{
    auto idx =
        static_cast<std::underlying_type<Policy>::type> (precisionPolicy);

    return productDecimalPlaces_[idx] (
        factor1DecimalPlaces,
        factor2DecimalPlaces,
        maxInternalDecimalPlaces
    );
}

inline unsigned int Precision::getQuotientDecimalPlaces (
    const unsigned int dividendDecimalPlaces,
    const unsigned int divisorDecimalPlaces,
    const unsigned int maxInternalDecimalPlaces,
    const Policy precisionPolicy
)
{
    auto idx =
        static_cast<std::underlying_type<Policy>::type> (precisionPolicy);

    return quotientDecimalPlaces_[idx] (
        dividendDecimalPlaces,
        divisorDecimalPlaces,
        maxInternalDecimalPlaces
    );
}

inline const std::string& Precision::policyToString (Policy policy)
{
    auto idx = static_cast<std::underlying_type<Policy>::type> (policy);

    return policyStrings_[idx];
}

} // namespace fixed

#endif // FIXED_PRECISION_H
