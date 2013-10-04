#ifndef FIXED_NUMBER_H
#define FIXED_NUMBER_H

#include "fixed/Absolute.h"
#include "fixed/Exceptions.h"
#include "fixed/FirstBitSet.h"
#include "fixed/Precision.h"
#include "fixed/Rounding.h"
#include "fixed/ShiftTable.h"

#include <cstdint>
#include <cmath>
#include <functional>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

//
// The maximum number we support representing is:
//
// 9223372036854775807.99999999999999
//
// The mininum is:
//
// -9223372036854775807.99999999999999
//
// The integer magnitude there is 2^63 - 1, and the number of decimal places is
// 14.  If the results of any additions, subtractions or multiplications would
// cause us to have a larger integer value a fixed::OverflowException will be
// thrown.
//
// When doing multiplication, internally we may have to lower the precision on
// the factors in order have room to store the results.  When doing division
// the maximum precision we can offer for the result may need to be limited
// depending on the magnitude of the dividend and the precision of the divisor.
//
// Here are two extreme examples to illustrate what this means:
//
// E.g. 1
//
// 123456789012.12345678901234 * 74709314.17104198834225
//
//   True result, rounded to 14 decimal places:
//
//     9223372036854775806.99899284895878
//
//   In practice, what this library does is the following multiplication:
//
//     123456789012.12345679 * 74709314.17104198834
//       = 9223372036854775806.79500247491567
//
// E.g. 2
//
// 3676299675362152112.41203440812031 / 0.39858520947355
//
//   True result, rounded to 14 decimal places
//
//     9223372036854544405.23297216736970
//
//   In practice, this library can only offer up to 5 decimal places of
//   precision in this case:
//
//     9223372036854544405.23297
//
// These limitations only come into play when dealing with large numbers, in
// the context of normal values used in financial settings they shouldn't pose
// a real concern.
//

namespace fixed {

class Number {
  public:
    enum class Sign {
        NEGATIVE = 0,
        POSITIVE
    };

    //
    // IMPORTANT, if fractionalValue is non-zero, then decimalPlaces must also
    // be non-zero; otherwise the validation check will fail and an exception
    // is thrown.
    //
    // A fixed::BadValueException will be thrown in the following cases:
    //   - If the magnitude of integerValue passed exceeds MAX_INTEGER_VALUE
    //   - If the fractionalValue passed in is too large to be represented by
    //     the decimalPlaces value passed in.
    //   - If the decimalPlaces passed in exceeds MAX_DECIMAL_PLACES.
    //
    // The static utility function Number::validate () can be used to validate
    // these parameters before attempting to instantiate a Number, ie for
    // validating values passed in from a user or from a protobuf msg before
    // blindly using them.  If validate () returns true then the constructor is
    // guaranteed to not throw an exception.
    //
    template <typename T>
    explicit Number (
        const T& integerValue,
        const uint64_t fractionalValue = 0,
        const unsigned int decimalPlaces = 0,
        Sign sign = Sign::POSITIVE
    );

    //
    // Constructors taking in a float, double and long double are also
    // provided, however if possible the earlier version passing in an integral
    // type should be preferred as it provides more accuracy.
    //
    // If the desired decimalPlaces are not specified, then the min decimal
    // places required to accurately store the number will be used.  This means
    // any excess 0's at the end of the number will be trimmed if possible.
    // However if decimalPlaces is provided that will be the decimal places in
    // use.  If a decimalPlaces larger than MAX_DECIMAL_PLACES is passed in
    // this is considered the same as not specifying the desired decimal places
    // and then the min decimal places required will be used.
    //
    //
    // NOTE: Numbers that are smaller than our minimum representable value
    //       will simply result in a Number of 0.  The method isZero () can
    //       be queried to check for this after the fact.
    //
    // A fixed::BadValueException will be thrown in the following cases:
    //   - The magnitude of the integer part of the value passed in exceeds
    //     MAX_INTEGER_VALUE.
    //   - The value passed in is an 'infinite' type.
    //   - The value passed in is NaN.
    //
    explicit Number (
        const float val,
        unsigned int decimalPlaces = (MAX_DECIMAL_PLACES + 1),
        Rounding::Mode roundingMode = defaultRoundingMode_
    );

    explicit Number (
        const double val,
        unsigned int decimalPlaces = (MAX_DECIMAL_PLACES + 1),
        Rounding::Mode roundingMode = defaultRoundingMode_
    );

    explicit Number (
        const long double val,
        unsigned int decimalPlaces = (MAX_DECIMAL_PLACES + 1),
        Rounding::Mode roundingMode = defaultRoundingMode_
    );

    //
    // The default constructor can't throw an exception.
    //
    Number () noexcept;

    //
    // A fixed::BadValueException will be thrown in the following cases:
    //   - The integer value specified is larger than MAX_INTEGER_VALUE.
    //   - The number of decimal places specified is larger than
    //     MAX_DECIMAL_PLACES.
    //   - The string is badly formatted.
    //
    // Examples of valid strings:
    //
    // 7
    // 1.0
    // -0.01
    // 1.23456
    //
    explicit Number (const std::string& numberStr);
    explicit Number (const char* c_str);

    //
    // Policy to control how much precision is kept for the results of
    // multiplication.  The policies of the two operands will be compared, and
    // the one that leads to the most precision will be kept.
    //
    // This is the initial default value the library will use.  Clients can
    // override this default by calling setDefaultMultPrecisionPolicy () which
    // will then be used for all future instantiations of a Number.
    //
    // Clients can override the value on a specific instance of a Number by
    // calling setMultPrecisionPolicy () on that instance.
    //
    static constexpr Precision::Policy DEFAULT_MULT_PRECISION_POLICY =
        Precision::Policy::MAX_OPERAND_PLUS_2;

    //
    // Policy to control how much precision is kept for the results of
    // division.  The policies of the two operands will be compared, and the
    // one that leads to the most precision will be kept.
    //
    // This is the initial default value the library will use.  Clients can
    // override this default by calling setDefaultDivPrecisionPolicy () which
    // will then be used for all future instantiations of a Number.
    //
    // Clients can override the value on a specific instance of a Number by
    // calling setDivPrecisionPolicy () on that instance.
    //
    static constexpr Precision::Policy DEFAULT_DIV_PRECISION_POLICY =
        Precision::Policy::MAX_OPERAND_PLUS_2;

    //
    // This is the default rounding mode that will be used when we are reducing
    // the number of decimal places a Number has.
    //
    // This is the initial default value the library will use.  Clients can
    // override this default by calling setDefaultRoundingMode () which
    // will then be used for all future instantiations of a Number.
    //
    // Clients can override the value on a specific instance of a Number by
    // calling setRoundingMode () on that instance.
    //
    static constexpr Rounding::Mode DEFAULT_ROUNDING_MODE =
        Rounding::Mode::TO_NEAREST_HALF_TO_EVEN;

    //
    // This is the maximum magnitude for the integer portion of the Number
    // that will be supported.  Any operation that would cause the magnitude
    // of the integer portion to exceed this will cause a fixed::Overflow
    // exception to be thrown.
    //
    static constexpr uint64_t MAX_INTEGER_VALUE =
        std::numeric_limits<int64_t>::max();

    //
    // Upper bound on the number of decimal places we'll support, results
    // of divisions or muliplications that should have more precision will
    // be capped at this, and setDecimalPlaces () will throw an exception if
    // the value passed in is greater than this.
    //
    static constexpr unsigned int MAX_DECIMAL_PLACES = 14;

    //
    // The maximum value for the fractional portion of the Number, which
    // follows directly from the MAX_DECIMAL_PLACES.
    //
    static constexpr uint64_t MAX_FRACTIONAL_VALUE =
        static_cast<uint64_t>(pow(10, MAX_DECIMAL_PLACES)) - 1;

    //
    // When doing division, we can insert extra precision in the computation so
    // that when adjusting the quotient to the target number of decimal places,
    // it is rounding correctly.
    //
    static constexpr unsigned int DIVISION_EXTRA_DP_FOR_ROUNDING = 1;

    static constexpr char STRING_OUTPUT_DECIMAL_SEPARATOR = '.';

    //
    // Verifies if the values are all sensical and can be used to construct
    // a Number, clients can validate data received via protobuf etc by calling
    // this and if this returns true the constructor is guaranteed to not
    // throw an exception.
    //
    template <typename T>
    static bool validate (
        const T& integerValue,
        const uint64_t fractionalValue,
        const unsigned int decimalPlaces,
        const Sign sign
    ) noexcept;

    //
    // Retrieves the magnitude of the integer value of the Number.  Note
    // the return value is unsigned.  In order to check for a negative
    // Number you must call isNegative ().  Two reasons for this, the first is
    // it allows us to support a max integer value of uint64::max if desired,
    // and two, if the integer value is 0 we can't return -0, so no need
    // to return a negative value period.
    //
    // Examples:
    //   Returns 0 for the Number -0.51
    //   Returns 1 for the Number -1.51
    //   Returns 2 for the Number 2.51
    //
    uint64_t integerValue () const noexcept;

    //
    // Retrives the fractional component of the Number.
    //
    // Examples:
    //   Returns 51 for the Number -0.51
    //   Returns 0 for the Number 10.0
    //   Returns 0 for the Number 10
    //
    uint64_t fractionalValue () const noexcept;

    //
    // Returns the number of decimal places for Number.
    //
    // Examples:
    //   Returns 0 for the Number 10
    //   Returns 1 for the Number 10.0
    //   Returns 2 for the Number 10.21
    //
    unsigned int decimalPlaces () const noexcept;

    //
    // Returns true if the Number is negative, false otherwise.
    //
    bool isNegative () const noexcept;

    //
    // Returns true if the Number is positive, false otherwise.  Zero does not
    // count as positive.
    //
    bool isPositive () const noexcept;

    //
    // Returns true if the Number is zero.
    //
    bool isZero () const noexcept;

    //
    // Returns true if internally the Number is using a int64_t value for
    // storage, as opposed to the more expensive __int128_t
    //
    bool value64Set () const noexcept;

    //
    // Modifies the number of decimal places used by the Number.  In the
    // event the number of decimal places is being reduced the current
    // rounding policy in effect will be used to round the value of the digit
    // in the last decimal place being kept.
    //
    // Will throw fixed::BadValueException if the desired decimalPlaces is
    // larger than MAX_DECIMAL_PLACES
    //
    void setDecimalPlaces (unsigned int targetDecimalPlaces);

    //
    // The +=, -=, *= and =/ operators can throw fixed::OverflowException.
    //
    // The /= and %= operators can throw a fixed::DivideByZeroException.
    //
    // The %= operator will update the Number to be the remainder of x / y,
    // more specifically it does the following:
    //
    //   x - ny
    //
    // Where n is the the rounded towards zero integer value of x / y.
    //
    Number& operator+= (const Number& rhs);
    Number& operator-= (const Number& rhs);
    Number& operator*= (const Number& rhs);
    Number& operator/= (const Number& rhs);
    Number& operator%= (const Number& rhs);

    //
    // Returns a string representation of the number.  This number will include
    // the number of decimal places currently in use.
    //
    std::string toString () const noexcept;

    //
    // Applies an absolute value function to the current Number (ie makes
    // it positive if was negative, keeping the number itself unchanged).
    //
    Number& toAbsolute () noexcept;

    //
    // Returns a new number that is the absolute value of the number passed in.
    //
    static Number toAbsolute (const Number& n) noexcept;

    //
    // Return the number represented as a double and long double respectively
    //
    double toDouble () const noexcept;
    long double toLongDouble () const noexcept;

    //
    // Modifies the default multiplication precision policy at the library
    // level, all future instantiations of a Number will use this value.
    //
    // See comment for DEFAULT_MULT_PRECISION_POLICY for more on the
    // mulicplication precision policy.
    //
    static void setDefaultMultPrecisionPolicy (
        const Precision::Policy& policy
    ) noexcept;

    //
    // Modifies the default division precision policy at the library
    // level, all future instantiations of a Number will use this value.
    //
    // See comment for DEFAULT_DIVISION_PRECISION_POLICY for more on the
    // division precision policy.
    //
    static void setDefaultDivPrecisionPolicy (
        const Precision::Policy& policy
    ) noexcept;

    //
    // Modifies the default rounding mode at the library level, all future
    // instantiations of a Number will use this value.
    //
    // See comment for DEFAULT_ROUNDING_MODE for more on the rounding mode.
    //
    static void setDefaultRoundingMode (
        const Rounding::Mode& mode
    ) noexcept;

    //
    // Updates the multiplication precision policy for this specific instance
    // of the Number.
    //
    void setMultPrecisionPolicy (const Precision::Policy& policy) noexcept;

    //
    // Updates the division precision policy for this specific instance
    // of the Number.
    //
    void setDivPrecisionPolicy (const Precision::Policy& policy) noexcept;

    //
    // Updates the division precision policy for this specific instance
    // of the Number.
    //
    void setRoundingMode (const Rounding::Mode& mode) noexcept;

    //
    // Returns the rounding mode for this specific instance of the Number.
    //
    Rounding::Mode roundingMode () const noexcept;

    //
    // Removes upto maxSqueeze zeros from the right hand side of the value
    // passed in.
    //
    template <typename T>
    static unsigned int squeezeZeros (
        T& val,
        unsigned int maxSqueeze = std::numeric_limits<T>::digits10
    ) noexcept;

  private:

    static constexpr long double MAX_LONG_DOUBLE_VAL = (
        static_cast<long double> (MAX_INTEGER_VALUE) +
        (
            static_cast<long double> (MAX_FRACTIONAL_VALUE) /
            powl (10, MAX_DECIMAL_PLACES)
        )
    );

    void initSetValue (
        const uint64_t integerValue,
        const uint64_t fractionalValue,
        const unsigned int decimalPlaces,
        const Sign sign
    );

    template <typename T>
    static void setValue (
        const uint64_t integerValue,
        const uint64_t fractionalValue,
        const unsigned int decimalPlaces,
        const Sign sign,
        T& value
    );

    //
    // Determines if there are any trailing zeros after the decimal point that
    // can simply be removed for the purpose of keeping numbers smaller
    // for multiplication and not having to lose precision.
    //
    bool isCompact () const noexcept;

    //
    // Will remove all trailing zeros to get a more compact representation
    // of the number.
    //
    // Returns the number of decimal places that were removed.
    //
    unsigned int makeCompact (
        const unsigned int maxDpReduce =
            std::numeric_limits<__int128_t>::digits10
    ) noexcept;

    template <typename T>
    static uint64_t integerValue (
        const T& val,
        const unsigned int decimalPlaces
    ) noexcept;

    template <typename T>
    uint64_t fractionalValue (const T& val) const noexcept;

    template <typename T>
    T toFloatingPoint () const noexcept;

    friend bool operator== (const Number& lhs, const Number& rhs);
    friend bool operator< (const Number& lhs, const Number& rhs);
    friend bool operator<= (const Number& lhs, const Number& rhs);

    friend const Number operator+ (const Number& lhs, const Number& rhs);
    friend const Number operator- (const Number& lhs, const Number& rhs);
    friend const Number operator* (const Number& lhs, const Number& rhs);
    friend const Number operator/ (const Number& lhs, const Number& rhs);

    friend const Number operator% (const Number& lhs, const Number& rhs);

    template <typename T> bool isCompact (const T& val) const noexcept;

    template <typename T> unsigned int makeCompact (
        T& val,
        const unsigned int maxDpReduce
    ) noexcept;

    //
    // If we're currently a 128 bit value we'll attempt to switch to 64 bit if
    // possible, will also switch from a 64bit value to 128bit one if we're
    // at int64::min, certain things rely on this.
    //
    void valueAutoResize () noexcept;

    void upsizeTo128 () noexcept;

    void increaseDecimalPlaces64 (unsigned int targetDecimalPlaces) noexcept;
    void increaseDecimalPlaces128 (unsigned int targetDecimalPlaces) noexcept;

    void decreaseDecimalPlaces64 (unsigned int targetDecimalPlaces) noexcept;
    void decreaseDecimalPlaces128 (unsigned int targetDecimalPlaces) noexcept;

    typedef std::function<
        int64_t (const int64_t& v1, const int64_t& v2)
    > AddSubOperation64;

    typedef std::function<
        __int128_t (const __int128_t& v1, const __int128_t& v2)
    > AddSubOperation128;

    Number& addSub (
        const Number& rhs,
        AddSubOperation64 arithop64,
        AddSubOperation128 arithop128
    );

    typedef std::function<
        bool (const int64_t& v1, const int64_t& v2)
    > RelationalOperation64;

    typedef std::function<
        bool (const __int128_t& v1, const __int128_t& v2)
    > RelationalOperation128;

    //
    // Only to be called if the decimal places of the two Numbers are equal
    //
    static bool relationalValuesCmp (
        const Number& lhs,
        const Number& rhs,
        RelationalOperation64 relop64,
        RelationalOperation128 relop128
    ) noexcept;

    static bool relationalOperation (
        const Number& lhs,
        const Number& rhs,
        RelationalOperation64 relop64,
        RelationalOperation128 relop128
    ) noexcept;

    //
    // Utility function meant for Number::Number (std::string)
    //
    unsigned long long convertStrToVal (
        const char* cptr,
        char*& endptr,
        Sign& sign,
        const std::string& errMsgHeader
    );

    //
    // Can throw fixed::OverflowException
    //
    template <typename T> static T addition (const T& v1, const T& v2);

    //
    // Can throw fixed::OverflowException
    //
    template <typename T> static T subtraction (const T& v1, const T& v2);

    template <typename T> static bool isNegative (const T& value) noexcept;

    template <typename T> static bool isPositive (const T& value) noexcept;

    template <typename T> static bool isZero (const T& value) noexcept;

    template <typename T>
    unsigned int increaseDecimalPlacesBitCount (
        const T& value,
        unsigned int targetDecimalPlaces
    ) noexcept;

    bool integerValueOverflowCheck ();

    static bool integerValueOverflowCheck (
        __int128_t value,
        unsigned int decimalPlaces
    );

    //
    // Can throw fixed::OverflowException
    //
    Number& mult (const Number& rhs);
    void mult64 (const Number& rhs, unsigned int& resultingDecimalPlaces);

    void mult128 (
        Number rhs, // note by value
        unsigned int& resultingDecimalPlaces
    );

    //
    // In order for us to be able to store the result of a multiplication
    // we may need to lower the precision of the operands.
    //
    void multReducePrecision (
        const unsigned int excessBits,
        Number& n1,
        Number& n2
    );

    Number& div (const Number& rhs);

    void div64 (
        const Number& rhs,
        unsigned int& targetDecimalPlaces,
        unsigned int& requiredDividendShift,
        unsigned int& excessDividendShift
    );

    void div128 (
        Number rhs, // note, by value
        unsigned int& targetDecimalPlaces,
        unsigned int& requiredDividendShift,
        unsigned int& excessDividendShift
    );

    //
    // Can throw DivideByZeroException
    //
    Number& remainder (const Number& rhs);

    void remainderEqualDecimalPlaces (const Number& lhs) noexcept;

    Precision::Policy multPrecisionPolicy_;

    Precision::Policy divPrecisionPolicy_;

    Rounding::Mode roundingMode_;

    uint8_t decimalPlaces_;

    //
    // Ideally we'll always use the 64 bit value, and fallback to the 128
    // bit representation if we have to, since the math for it is emulated
    // it's slower.
    //
    bool value64Set_;

    static_assert (
        std::numeric_limits<__int128_t>::is_specialized,
        "Need to compile with -std=c++11 -U__STRICT_ANSI__ "
        "in order to get __int128_t type_trait and numeric_limits support"
    );

    union {
        int64_t value64_;
        __int128_t value128_;
    };

    static const FirstBitSet firstBitSet_;

    static const ShiftTable<int64_t> shiftTable64_;

    //
    // Only used when required, shiftTable64_ is used whenever possible by
    // the code.
    //
    static const ShiftTable<__int128_t> shiftTable128_;

    static Precision::Policy defaultMultPrecisionPolicy_;
    static Precision::Policy defaultDivPrecisionPolicy_;
    static Rounding::Mode    defaultRoundingMode_;
};

//
// The +, -, * and / operators can throw fixed::OverflowException.
//
// The / and % operators can throw a fixed::divideByZeroException.
//
// The % operator returns the remainder of x / y, more specifically it does
// the following:
//
//   x - ny
//
// Where n is the the rounded towards zero integer value of x / y.
//
const Number operator+ (const Number& lhs, const Number& rhs);
const Number operator- (const Number& lhs, const Number& rhs);
const Number operator* (const Number& lhs, const Number& rhs);
const Number operator/ (const Number& lhs, const Number& rhs);
const Number operator% (const Number& lhs, const Number& rhs);

bool operator<  (const Number& lhs, const Number& rhs);
bool operator<= (const Number& lhs, const Number& rhs);
bool operator>  (const Number& lhs, const Number& rhs);
bool operator>= (const Number& lhs, const Number& rhs);
bool operator== (const Number& lhs, const Number& rhs);
bool operator!= (const Number& lhs, const Number& rhs);

template <typename T>
T& operator<< (T& out, const Number& n);

template <typename T>
inline Number::Number (
    const T& integerValueT,
    const uint64_t fractionalValue,
    const unsigned int decimalPlaces,
    Sign sign
)
  : multPrecisionPolicy_ (defaultMultPrecisionPolicy_),
    divPrecisionPolicy_ (defaultDivPrecisionPolicy_),
    roundingMode_ (defaultRoundingMode_),
    decimalPlaces_ (decimalPlaces),
    value64Set_ (true),
    value64_ (0)
{
    static_assert (
        std::is_integral<T>::value && sizeof (T) <= 8,
        "This constructor only supports integral types of size 8 bytes or less "
        "for the integer value"
    );

    if (! validate (
            integerValueT,
            fractionalValue,
            decimalPlaces,
            sign
          )
       )
    {
        throw fixed::BadValueException ("Number::Number");
    }

    uint64_t integerValue = 0;

    if (std::is_signed<T>::value)
    {
        integerValue = absoluteValue<uint64_t> (integerValueT);

        //
        // Note, we override the negative flag only if the integerValue passed
        // in was negative.
        //
        if (isNegative (integerValueT))
        {
            sign = Sign::NEGATIVE;
        }
    }
    else
    {
        integerValue = static_cast<uint64_t> (integerValueT);
    }

    initSetValue (integerValue, fractionalValue, decimalPlaces, sign);
}

inline Number::Number (
    const float val,
    unsigned int decimalPlaces,
    Rounding::Mode roundingMode
)
  : Number (static_cast<long double> (val), decimalPlaces, roundingMode)
{
}

inline Number::Number (
    const double val,
    unsigned int decimalPlaces,
    Rounding::Mode roundingMode
)
  : Number (static_cast<long double> (val), decimalPlaces, roundingMode)
{
}

inline Number::Number (
    const long double val,
    unsigned int decimalPlaces,
    Rounding::Mode roundingMode
)
  : multPrecisionPolicy_ (defaultMultPrecisionPolicy_),
    divPrecisionPolicy_ (defaultDivPrecisionPolicy_),
    roundingMode_ (roundingMode),
    decimalPlaces_ (0),
    value64Set_ (true),
    value64_ (0)
{
    if (std::isnan (val))
    {
        throw fixed::BadValueException (
            "Floating point constructor value is not a number"
        );
    }

    if (std::isinf (val))
    {
        throw fixed::BadValueException (
            "Floating point constructor value is + or - infinity"
        );
    }

    bool minimizeDps = false;

    if (decimalPlaces > MAX_DECIMAL_PLACES)
    {
        minimizeDps = true;
        decimalPlaces = MAX_DECIMAL_PLACES;
    }

    //
    // Note, passing in fabsl (val) into modf, this way both intPart and
    // fractPart will be positive, will simplify things.
    //
    long double intPart = 0.0;
    long double fractPart = std::modf (fabsl (val), &intPart);

    if (intPart > static_cast<long double> (MAX_INTEGER_VALUE))
    {
        throw fixed::BadValueException (
            "Floating point constructor, integer value too large."
        );
    }

    //
    // Note, initially use MAX_DECIMAL_PLACES, then afterwards we'll do a
    // setDecimalPlaces to get the desired precision and rounding.
    //
    decimalPlaces_ = MAX_DECIMAL_PLACES;

    initSetValue (
        static_cast<uint64_t> (intPart),
        static_cast<uint64_t> (fractPart * shiftTable64_[decimalPlaces_].value),
        MAX_DECIMAL_PLACES,
        (val < 0.0) ? Sign::NEGATIVE : Sign::POSITIVE
    );


    if (minimizeDps)
    {
        makeCompact ();
    }
    else
    {
        setDecimalPlaces (decimalPlaces);
    }
}

inline Number::Number () noexcept
  : Number (0)
{
}

inline Number::Number (const std::string& numberString)
  : Number (numberString.c_str ())
{
}

//
// Note, this is templatized for integerValue since it allows us to support
// having the MAX_INTEGER_VALUE be uint64_t::max.  We also want to support
// doing Number (-3), so we couldn't make the integerValue parameter be a
// uint64_t to satisfy both requirements, therefore we templatized it.
//
template <typename T>
bool Number::validate (
    const T& integerValue,
    const uint64_t fractionalValue,
    const unsigned int decimalPlaces,
    const Sign sign
) noexcept
{
    static_assert (
        std::is_integral<T>::value && sizeof (T) <= 8,
        "This validate () only supports integral types of size 8 bytes or less "
        "for the integer value."
    );

    return (
        (absoluteValue<uint64_t> (integerValue) <= MAX_INTEGER_VALUE) &&
        (decimalPlaces <= MAX_DECIMAL_PLACES) &&
        (fractionalValue <
            static_cast<uint64_t> (shiftTable64_[decimalPlaces].value))
    );
}

inline void Number::initSetValue (
    const uint64_t integerValue,
    const uint64_t fractionalValue,
    const unsigned int decimalPlaces,
    const Sign sign
)
{
    unsigned int bitsSum =
        firstBitSet_ (integerValue) + shiftTable64_[decimalPlaces].firstBitSet;

    if (bitsSum > FirstBitSet::maxBitPos<int64_t> ())
    {
        setValue<__int128_t> (
            integerValue,
            fractionalValue,
            decimalPlaces,
            sign,
            value128_
        );

        value64Set_ = false;

        //
        // The heuristic for the overflow is not exact, once we've done it it's
        // possible we could still fit in a 64 bit...
        //
        valueAutoResize ();
    }
    else
    {
        setValue<int64_t> (
            integerValue,
            fractionalValue,
            decimalPlaces,
            sign,
            value64_
        );

        value64Set_ = true;
    }
}

template <typename T>
inline void Number::setValue (
    const uint64_t integerValue,
    const uint64_t fractionalValue,
    const unsigned int decimalPlaces,
    const Sign sign,
    T& value
)
{
    value = static_cast<T> (integerValue);
    value *= shiftTable64_[decimalPlaces].value;
    value += fractionalValue;

    if (sign == Sign::NEGATIVE)
    {
        value = - value;
    }
}

inline uint64_t Number::integerValue () const noexcept
{
    return value64Set_ ?
            integerValue<int64_t> (value64_, decimalPlaces ()) :
            integerValue<__int128_t> (value128_, decimalPlaces ())
    ;
}

template <typename T>
inline uint64_t Number::integerValue (
    const T& val,
    const unsigned int decimalPlaces
) noexcept
{
    T intVal = val / shiftTable64_[decimalPlaces].value;

    //
    // Even for the case when T is int128, the casts to uint64_t below will
    // always work and not cause any truncated data.  Internally the max
    // integer value we support is the max a uint64_t can hold, all operations
    // that would cause it to be larger will cause an overflow exception.
    //
    return isNegative<T> (intVal) ?
           static_cast<uint64_t> (- intVal) :
           static_cast<uint64_t> (intVal)
    ;
}

inline uint64_t Number::fractionalValue () const noexcept
{
    return value64Set_ ?
            fractionalValue<int64_t> (value64_) :
            fractionalValue<__int128_t> (value128_)
    ;
}

template <typename T>
inline uint64_t Number::fractionalValue (const T& val) const noexcept
{
    //
    // The result of the mod will always fit in 60bits or less, since
    // MAX_DECIMAL_PLACE is <= 18 (from the fundamentalAssumptions () check),
    // so even when T is int128 the casting will not cause any truncation.
    //
    return (
        fixed::absoluteValue<uint64_t> (
            static_cast<int64_t> (val % shiftTable64_[decimalPlaces ()].value)
        )
    );
}

inline unsigned int Number::decimalPlaces () const noexcept
{
    return static_cast<unsigned int> (decimalPlaces_);
}

inline bool Number::isNegative () const noexcept
{
    return (
        value64Set_ ?
            isNegative (value64_) :
            isNegative (value128_)
    );
}

inline bool Number::isPositive () const noexcept
{
    return (
        value64Set_ ?
            isPositive (value64_) :
            isPositive (value128_)
    );
}

inline bool Number::isZero () const noexcept
{
    return (
        value64Set_ ?
            isZero (value64_) :
            isZero (value128_)
    );
}

inline bool Number::value64Set () const noexcept
{
    return value64Set_;
}

template <typename T>
unsigned int Number::squeezeZeros (
    T& val,
    unsigned int maxSqueeze
) noexcept
{
    if (! val)
    {
        return 0;
    }

    int idx = 0;
    unsigned int numSqueezed = 0;

    while ((shiftTable64_[idx + 1].decimalPlaces <= maxSqueeze) &&
           ((val % shiftTable64_[idx + 1].value) == 0))
    {
        idx++;

        if (idx == MAX_DECIMAL_PLACES)
        {
            val /= shiftTable64_[MAX_DECIMAL_PLACES].value;
            numSqueezed += shiftTable64_[MAX_DECIMAL_PLACES].decimalPlaces;
            maxSqueeze -= shiftTable64_[MAX_DECIMAL_PLACES].decimalPlaces;
            idx = 0;
        }
    }

    if (idx)
    {
        val /= shiftTable64_[idx].value;
        numSqueezed += shiftTable64_[idx].decimalPlaces;
    }

    return numSqueezed;
}

template <typename T> bool Number::isPositive (const T& value) noexcept
{
    return (value > 0);
}

template <typename T> bool Number::isNegative (const T& value) noexcept
{
    return (value < 0);
}

template <typename T> bool Number::isZero (const T& value) noexcept
{
    return (value == 0);
}

inline void Number::setDefaultMultPrecisionPolicy (
    const Precision::Policy& policy
) noexcept
{
    defaultMultPrecisionPolicy_ = policy;
}

inline void Number::setDefaultDivPrecisionPolicy (
    const Precision::Policy& policy
) noexcept
{
    defaultDivPrecisionPolicy_ = policy;
}

inline void Number::setMultPrecisionPolicy (
    const Precision::Policy& policy
) noexcept
{
    multPrecisionPolicy_ = policy;
}

inline void Number::setDivPrecisionPolicy (
    const Precision::Policy& policy
) noexcept
{
    divPrecisionPolicy_ = policy;
}

inline void Number::setDefaultRoundingMode (
    const Rounding::Mode& mode
) noexcept
{
    defaultRoundingMode_ = mode;
}

inline void Number::setRoundingMode (const Rounding::Mode& mode) noexcept
{
    roundingMode_ = mode;
}

inline Rounding::Mode Number::roundingMode () const noexcept
{
    return roundingMode_;
}

inline Number& Number::toAbsolute () noexcept
{
    if (value64Set_)
    {
        //
        // Note, this assumes that valueAutoResize () ensures int64::min
        // is always stored in the value128_
        //
        value64_ = absoluteValue<int64_t> (value64_);
    }
    else
    {
        value128_ = absoluteValue<__int128_t> (value128_);
    }

    return *this;
}

//
// Returns a new number that is the absolute value of the number passed in.
//
inline Number Number::toAbsolute (const Number& n) noexcept
{
    return Number (n).toAbsolute ();
}

inline double Number::toDouble () const noexcept
{
    return toFloatingPoint<double> ();
}

inline long double Number::toLongDouble () const noexcept
{
    return toFloatingPoint<long double> ();
}

template <typename T> inline T Number::toFloatingPoint () const noexcept
{
    T val = static_cast<T> (integerValue ());

    val += (
        static_cast<T> (fractionalValue ()) /
        static_cast<T> (shiftTable64_[decimalPlaces ()].value)
    );

    return isNegative () ? - val : val;
}

inline std::string Number::toString () const noexcept
{
    std::ostringstream os;

    os << *this;

    return os.str ();
}

template <typename T>
inline T& operator<< (T& out, const Number& n)
{
    if (n.isNegative ())
    {
        out << '-';
    }

    out << n.integerValue ();

    if (n.decimalPlaces ())
    {
        out << Number::STRING_OUTPUT_DECIMAL_SEPARATOR
            << std::setw (n.decimalPlaces ()) << std::setfill ('0')
            << n.fractionalValue ();
    }

    return out;
}

} // namespace fixed

#endif // FIXED_NUMBER_H
