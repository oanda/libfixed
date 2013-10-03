#include "fixed/Precision.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace fixed {

const std::vector<std::string> Precision::policyStrings_ = {
  {
    "MIN_OPERAND",
    "MIN_OPERAND_PLUS_1",
    "MIN_OPERAND_PLUS_2",
    "MIN_OPERAND_PLUS_3",
    "MIN_OPERAND_PLUS_4",
    "MIN_OPERAND_PLUS_5",
    "MAX_OPERAND",
    "MAX_OPERAND_PLUS_1",
    "MAX_OPERAND_PLUS_2",
    "MAX_OPERAND_PLUS_3",
    "MAX_OPERAND_PLUS_4",
    "MAX_OPERAND_PLUS_5",
    "MAX_PRECISION"
  }
};

const Precision::RunTimePolicyStringsCheck
    Precision::runTimePolicyStringsCheck_;

template <unsigned int VAL>
unsigned int minOperandProduct (
    const unsigned int factor1DecimalPlaces,
    const unsigned int factor2DecimalPlaces,
    const unsigned int maxInternalDecimalPlaces
)
{
    return std::min (
        std::min (
            std::min (factor1DecimalPlaces, factor2DecimalPlaces) + VAL,
            factor1DecimalPlaces + factor2DecimalPlaces
        ),
        maxInternalDecimalPlaces
    );
}

template <unsigned int VAL>
unsigned int maxOperandProduct (
    const unsigned int factor1DecimalPlaces,
    const unsigned int factor2DecimalPlaces,
    const unsigned int maxInternalDecimalPlaces
)
{
    return std::min (
        std::min (
            std::max (factor1DecimalPlaces, factor2DecimalPlaces) + VAL,
            factor1DecimalPlaces + factor2DecimalPlaces
        ),
        maxInternalDecimalPlaces
    );
}

template <unsigned int VAL>
unsigned int minOperandQuotient (
    const unsigned int dividendDecimalPlaces,
    const unsigned int divisorDecimalPlaces,
    const unsigned int maxInternalDecimalPlaces
)
{
    return std::min (
        std::min (dividendDecimalPlaces, divisorDecimalPlaces) + VAL,
        maxInternalDecimalPlaces
    );
}

template <unsigned int VAL>
unsigned int maxOperandQuotient (
    const unsigned int dividendDecimalPlaces,
    const unsigned int divisorDecimalPlaces,
    const unsigned int maxInternalDecimalPlaces
)
{
    return std::min (
        std::max (dividendDecimalPlaces, divisorDecimalPlaces) + VAL,
        maxInternalDecimalPlaces
    );
}

unsigned int productMaxInternalPrecision (
    const unsigned int factor1DecimalPlaces,
    const unsigned int factor2DecimalPlaces,
    const unsigned int maxInternalDecimalPlaces
)
{
    return std::min (
        factor1DecimalPlaces + factor2DecimalPlaces,
        maxInternalDecimalPlaces
    );
}

unsigned int quotientMaxInternalPrecision (
    const unsigned int, // dividendDecimalPlaces
    const unsigned int, // divisorDecimalPlaces
    const unsigned int maxInternalDecimalPlaces
)
{
    return maxInternalDecimalPlaces;
}

const std::vector<
    std::function<
        unsigned int (
            const unsigned int operand1DecimalPlaces,
            const unsigned int operand2DecimalPlaces,
            const unsigned int maxInternalDecimalPlaces
        )
    >
> Precision::productDecimalPlaces_ = {
    minOperandProduct<0>,
    minOperandProduct<1>,
    minOperandProduct<2>,
    minOperandProduct<3>,
    minOperandProduct<4>,
    minOperandProduct<5>,
    maxOperandProduct<0>,
    maxOperandProduct<1>,
    maxOperandProduct<2>,
    maxOperandProduct<3>,
    maxOperandProduct<4>,
    maxOperandProduct<5>,
    productMaxInternalPrecision
};

const std::vector<
    std::function<
        unsigned int (
            const unsigned int operand1DecimalPlaces,
            const unsigned int operand2DecimalPlaces,
            const unsigned int maxInternalDecimalPlaces
        )
    >
> Precision::quotientDecimalPlaces_ = {
    minOperandQuotient<0>,
    minOperandQuotient<1>,
    minOperandQuotient<2>,
    minOperandQuotient<3>,
    minOperandQuotient<4>,
    minOperandQuotient<5>,
    maxOperandQuotient<0>,
    maxOperandQuotient<1>,
    maxOperandQuotient<2>,
    maxOperandQuotient<3>,
    maxOperandQuotient<4>,
    maxOperandQuotient<5>,
    quotientMaxInternalPrecision
};

Precision::RunTimePolicyStringsCheck::RunTimePolicyStringsCheck ()
{
    size_t expectedSize =
        static_cast<size_t> (
            static_cast<std::underlying_type<Policy>::type> (
                Policy::POLICY_MAX_VAL
            )
        );

    assert (expectedSize == policyStrings_.size ());
}

} // namespace fixed
