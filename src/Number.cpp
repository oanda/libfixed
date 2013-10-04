#include "fixed/FirstBitSet.h"
#include "fixed/Number.h"
#include "Utils.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ostream>

namespace fixed {

const FirstBitSet Number::firstBitSet_;

const ShiftTable<int64_t> Number::shiftTable64_ (MAX_INTEGER_VALUE);

const ShiftTable<__int128_t> Number::shiftTable128_ (MAX_INTEGER_VALUE);

Precision::Policy Number::defaultMultPrecisionPolicy_ =
    Number::DEFAULT_MULT_PRECISION_POLICY;

Precision::Policy Number::defaultDivPrecisionPolicy_ =
    Number::DEFAULT_DIV_PRECISION_POLICY;

Rounding::Mode Number::defaultRoundingMode_ = DEFAULT_ROUNDING_MODE;

//
// Notes on the static check on fundamentalAssumptions and on
// MAX_DECIMAL_PLACES and MAX_INTEGER_VALUE.  Parts of the code were written
// with these assumptions in mind, meaning certain checks in the code don't
// need to be performed etc.
//
// The values intially chosen for these of 18 and max value of a uint64_t
// respectively work out nicely.  When needed we'll use up to a __int128_t
// representation. The max magnitude number we can store is 127 bits (since
// we're using signed 128).  18 for max decimal places requires no more than 60
// bits to store, and coupled with the max unsigned integer value of 64 bits,
// adding 1 for the signed bit, all valid internal values will require 125 bits
// or less, giving us a buffer of 3 bits.  This buffer of 3 bits actually comes
// in handy.  For multiplication we use a heuristic to determine if an overflow
// would occur.  We do this by finding the most significant bits set in each
// operand and add them together.  This gives us the max possible bit set in
// the would be result, and we use this to check for overflow.  This is a worst
// case result however, it's possible it overestimates by up to 2 bits.  So the
// 3 bit buffer helps in that we won't mistakenly throw an overflow exception
// and ensures the results of all multiplications that we should be able to
// represent with our number range, we actually can represent despite this
// inexact overflow heuristic.  This also helps with the relational operators
// code, which may need to shift a number to do a proper comparison, again we
// don't have to worry about this heuristic causing us to not be able to shift
// a value into the correct range, since this shift is a multiplication by a
// power of 10, we'd have to use the same heuristic.
//
// The extra check for MAX_INTEGER_VALUE >= int64::max means we don't have to
// check for integer overflow on results of operations that still fit in an
// int64_t.  This is because even with a decimal places of 0, a result that
// fits in int64_t can't be > MAX_INTEGER_VALUE.
//
// MAX_DECIMAL_PLACES check on ShiftTable<int64>::MAX_DIGITS is because there's
// an assumption that all decimal places <= MAX_DECIMAL_PLACES can be used to
// index into shiftTable64_
//
// The 2 * MAX_DECIMAL_PLACES <= ShiftTable<int128_t>::MAX_DIGITS is because
// the worst case decimal places for the result of a multiplication is 2 *
// MAX_DECIMAL_PLACES, and we need to ensure we can index into shiftTable128_
// with this.
//
static constexpr bool fundamentalAssumptions ()
{
    return (
        (Number::MAX_DECIMAL_PLACES <= 18)
        &&
        (Number::MAX_INTEGER_VALUE <= std::numeric_limits<uint64_t>::max ())
        &&
        (Number::MAX_INTEGER_VALUE >= std::numeric_limits<int64_t>::max ())
        &&
        (
            Number::MAX_DECIMAL_PLACES <= ShiftTable<int64_t>::MAX_DIGITS
        )
        &&
        (
            2 * Number::MAX_DECIMAL_PLACES <= ShiftTable<__int128_t>::MAX_DIGITS
        )
    );
}

static_assert (
    fundamentalAssumptions (),
    "Updated constant values have caused some required fundamental "
    "assumptions to be broken that will cause the code to be incorrect."
);

Number::Number (const char* numberCStr)
  : multPrecisionPolicy_ (defaultMultPrecisionPolicy_),
    divPrecisionPolicy_ (defaultDivPrecisionPolicy_),
    roundingMode_ (defaultRoundingMode_),
    decimalPlaces_ (0),
    value64Set_ (true),
    value64_ (0)
{
    const char* cptr = numberCStr;

    Sign numberSign = Sign::POSITIVE;
    unsigned long long integerValue = 0;
    unsigned long long fractionalValue = 0;

    char* endptr;

    integerValue = convertStrToVal (
        cptr, endptr, numberSign, "Number::Number (str) IntegerValue "
    );

    if (*endptr == '.')
    {
        cptr = endptr + 1;

        if (! isdigit (cptr[0]))
        {
            throw fixed::BadValueException (
                "Number::Number (str) FractionalValue does not start with "
                "digit"
            );
        }

        Sign fracSign = Sign::POSITIVE;

        fractionalValue = convertStrToVal (
            cptr, endptr, fracSign, "Number::Number (str) FractionalValue "
        );

        decimalPlaces_ = static_cast<uint8_t> (endptr - cptr);

        if (decimalPlaces () > MAX_DECIMAL_PLACES)
        {
            throw fixed::BadValueException (
                "Number::Number (str) FractionalValue too large"
            );
        }
    }

    if (endptr[0] != '\0')
    {
        throw fixed::BadValueException (
            "Number::Number (str) number did not end in a digit"
        );
    }

    if (integerValue > MAX_INTEGER_VALUE)
    {
        throw fixed::BadValueException (
            "Number::Number (str) IntegerValue too large"
        );
    }

    initSetValue (
        static_cast<uint64_t> (integerValue),
        fractionalValue,
        decimalPlaces (),
        numberSign
    );
}

bool Number::isCompact () const noexcept
{
    if (value64Set_)
    {
        return isCompact<int64_t> (value64_);
    }
    else
    {
        return isCompact<__int128_t> (value128_);
    }
}

template <typename T>
bool Number::isCompact (const T& val) const noexcept
{
    //
    // Our defition of compact is no trailing zeros after the decimal
    // place, so a decimal place of 0 implies compactness.
    //
    return ((decimalPlaces () == 0) || ((val % 10) != 0));
}

unsigned int Number::makeCompact (const unsigned int maxDpReduce) noexcept
{
    return (
        value64Set_ ?
            makeCompact (value64_, maxDpReduce) :
            makeCompact (value128_, maxDpReduce)
    );
}

template <typename T>
unsigned int Number::makeCompact (
    T& val,
    const unsigned int maxDpReduce
) noexcept
{
    unsigned int squeezed = (
        (val != 0) ?
            squeezeZeros (val, std::min (decimalPlaces (), maxDpReduce)) :
            std::min (decimalPlaces (), maxDpReduce)
    );

    decimalPlaces_ -= squeezed;
    valueAutoResize ();

    return squeezed;
}

void Number::valueAutoResize () noexcept
{
    //
    // A value of int64::min is a special case, we'll keep it stored as 128
    // bits, some other parts of the code relies on this, one of them being
    // the toAbsolute () method.
    //
    if (value64Set_)
    {
        if (value64_ == std::numeric_limits<int64_t>::min ())
        {
            upsizeTo128 ();
        }
    }
    else if (firstBitSet_ (value128_) <= FirstBitSet::maxBitPos<int64_t> ())
    {
        value64_    = static_cast<int64_t> (value128_);
        value64Set_ = true;
    }
}

void Number::upsizeTo128 () noexcept
{
    if (value64Set_)
    {
        value128_   = static_cast<__int128_t> (value64_);
        value64Set_ = false;
    }
}

//
// NOTE: This method also gets called by the multiplication function when
// to potentially reduce the decimalPlaces_.  It's possible the value of
// decimalPlaces_ when this gets called is actually greater than
// MAX_DECIMAL_PLACES.  Something to keep in mind, and to ensure we
// don't use decimalPlaces_ to directly index shiftTable64_.
//
void Number::setDecimalPlaces (const unsigned int targetDecimalPlaces)
{
    if (targetDecimalPlaces == decimalPlaces ())
    {
        return;
    }

    if (targetDecimalPlaces > MAX_DECIMAL_PLACES)
    {
        throw fixed::BadValueException (
            "Number::setDecimalPlaces () Decimal place exceeds max"
        );
    }

    if (targetDecimalPlaces > decimalPlaces ())
    {
        if (value64Set_)
        {
            increaseDecimalPlaces64 (targetDecimalPlaces);
        }
        else
        {
            increaseDecimalPlaces128 (targetDecimalPlaces);
        }
    }
    else if (targetDecimalPlaces < decimalPlaces ())
    {
        if (value64Set_)
        {
            decreaseDecimalPlaces64 (targetDecimalPlaces);
        }
        else
        {
            decreaseDecimalPlaces128 (targetDecimalPlaces);
        }
    }

    decimalPlaces_ = static_cast<uint8_t> (targetDecimalPlaces);

    valueAutoResize ();
}

void Number::increaseDecimalPlaces64 (
    unsigned int targetDecimalPlaces
) noexcept
{
    if (increaseDecimalPlacesBitCount<int64_t> (value64_, targetDecimalPlaces)
        > FirstBitSet::maxBitPos<int64_t> ())
    {
        upsizeTo128 ();
        increaseDecimalPlaces128 (targetDecimalPlaces);
    }
    else
    {
        value64_ *=
            shiftTable64_[targetDecimalPlaces - decimalPlaces ()].value;
    }
}

void Number::increaseDecimalPlaces128 (
    unsigned int targetDecimalPlaces
) noexcept
{
    //
    // Note, in theory we should check for a potential overflow here, however
    // if the fundamentalAssumptions () hold, and the targetDecimalPlaces
    // passed in has been checked to be in the valid range, (which, since this
    // is an internal utility function those checks should have been done) it
    // is not possible for an overflow to occur here.
    //
    static_assert (
        fundamentalAssumptions (),
        "increaseDecimalPlaces128 requires the fundamentalAssuptions () hold"
    );

    assert (targetDecimalPlaces <= MAX_DECIMAL_PLACES);

    value128_ *= shiftTable128_[targetDecimalPlaces - decimalPlaces ()].value;
}

template <typename T>
unsigned int Number::increaseDecimalPlacesBitCount (
    const T& val,
    unsigned int targetDecimalPlaces
) noexcept
{
    return (
        firstBitSet_ (val) +
        shiftTable64_[targetDecimalPlaces - decimalPlaces ()].firstBitSet
    );
}

void Number::decreaseDecimalPlaces64 (
    unsigned int targetDecimalPlaces
) noexcept
{
    //
    // See note in setDecimalPlace, it's possible for decimalPlaces_ to be
    // greater than MAX_DECIMAL_PLACES for a temporary time until we update
    // it, for instance the result of multiplying two Numbers with a lot
    // of decimal places, it will then use setDecimalPlaces to adjust to
    // the desired target number of decimal places, which calls this to
    // carry that out.
    //
    if ((decimalPlaces () - targetDecimalPlaces) > shiftTable64_.MAX_DIGITS)
    {
        upsizeTo128 ();
        return decreaseDecimalPlaces128 (targetDecimalPlaces);
    }

    const auto& sval = shiftTable64_[decimalPlaces () - targetDecimalPlaces];

    //
    // The use of absoluteValue<int64_t> below requires that
    // fundamentalAssumptions () holds
    //
    static_assert (
        fundamentalAssumptions (), "Require fundamentalAssumptions"
    );

    //
    // Even though passing in signed value for 2nd param, if it turns out
    // to be 0, need to also pass the negativFlag in that last arg to cover
    // this case.
    //
    value64_ = Rounding::round (
        roundingMode_,
        value64_ / sval.value,
        absoluteValue<int64_t> (value64_ % sval.value),
        sval.halfRangeVal,
        (value64_ < 0)
    );
}

void Number::decreaseDecimalPlaces128 (
    unsigned int targetDecimalPlaces
) noexcept
{
    //
    // Due to the results of multiplication potentially having a decimalPlaces
    // larger than MAX_DECIMAL_PLACES, we rely on the one of the checks
    // from fundamentalAssumptions () to ensure the indexing into
    // the shiftTable below is always safe.
    //
    static_assert (
        fundamentalAssumptions (),
        "decreaseDecimalPlaces128 relies on the fundamental assumptions"
    );

    const auto& sval = shiftTable128_[decimalPlaces () - targetDecimalPlaces];

    __int128_t origVal = value128_;
    unsigned int origDecimalPlaces = decimalPlaces ();

    //
    // Even though passing in signed value for 2nd param, if it turns out
    // to be 0, need to also pass the negativFlag in that last arg to cover
    // this case.
    //
    value128_ = Rounding::round (
        roundingMode_,
        value128_ / sval.value,
        absoluteValue<__int128_t> (value128_ % sval.value),
        sval.halfRangeVal,
        (value128_ < 0)
    );

    //
    // This handles a corner case where someones calls setDecimalPlaces and it
    // causes a round to occur that changes us from a valid Number to one
    // with an integerValue () == (MAX_INTEGER_VALUE + 1).  We don't want
    // to have setDecimalPlace throw an OverflowException, so instead we'll
    // simply adjust the rounding in this one very special case.
    //
    // Also note, we don't have to do this check for the value64 case, due
    // to the fundamental assumptions this corner case can't hit in that mode.
    //
    if (integerValueOverflowCheck (value128_, targetDecimalPlaces))
    {
         __int128_t origIntValAbs =
            absoluteValue<__int128_t> (
                integerValue (origVal, origDecimalPlaces)
            );

        if (origIntValAbs == MAX_INTEGER_VALUE)
        {
            value128_ += isNegative (origVal) ? 1 : -1;
        }
    }
}

Number& Number::operator+= (const Number& rhs)
{
    //
    // We make the extra copy in the event addSub throws an exception we
    // don't have to worry about leaving the object in an inconsistent state
    //
    Number number (*this);
    number.addSub (rhs, addition<int64_t>, addition<__int128_t>);

    *this = number;
    return *this;
}

Number& Number::operator-= (const Number& rhs)
{
    //
    // We make the extra copy in the event addSub throws an exception we
    // don't have to worry about leaving the object in an inconsistent state
    //
    Number number (*this);
    number.addSub (rhs, subtraction<int64_t>, subtraction<__int128_t>);

    *this = number;
    return *this;
}

//
// Note, thought about simply implementing this all in +=, and making
// subtraction be += -1 * rhs, however then subtraction becomes a bit more
// expensive than addition, and you have to check that rhs is not already the
// minimum number, as you can't multiply it by -1 since it has no positive
// counterpart in the same datatype range.
//
Number& Number::addSub (
    const Number& rhs,
    AddSubOperation64 arithop64,
    AddSubOperation128 arithop128
)
{
    Number rhsCopy (rhs);

    if (decimalPlaces () > rhs.decimalPlaces ())
    {
        rhsCopy.setDecimalPlaces (decimalPlaces ());
    }
    else if (decimalPlaces () < rhs.decimalPlaces ())
    {
        setDecimalPlaces (rhsCopy.decimalPlaces ());
    }

    bool need128 = false;

    if (! value64Set_ || ! rhsCopy.value64Set_)
    {
        need128 = true;
        upsizeTo128 ();
        rhsCopy.upsizeTo128 ();
    }

    if (! need128)
    {
        //
        // If the 64 bit attempt Overflows, we'll fall through and attempt
        // 128 bit
        //
        try {
            value64_ = arithop64 (value64_, rhsCopy.value64_);
        }
        catch (const fixed::OverflowException& e)
        {
            need128 = true;
            upsizeTo128 ();
            rhsCopy.upsizeTo128 ();
        }
    }

    if (need128)
    {
        //
        // We let overflow exceptions escape out of this one
        //
        value128_ = arithop128 (value128_, rhsCopy.value128_);
    }

    valueAutoResize ();

    if (integerValueOverflowCheck ())
    {
        throw fixed::OverflowException (
            "addSub: Addition or subtraction caused an overflow"
        );
    }

    return *this;
}

template <typename T> T Number::addition (const T& v1, const T& v2)
{
    static_assert (
        fundamentalAssumptions (),
        "Logic in addition () requires fundamentalAssuptions () hold"
    );

    static_assert (
        std::is_same<T, int64_t>::value || std::is_same<T, __int128_t>::value,
        "Expecting int64_t or int128_t"
    );

    T result = v1 + v2;

    //
    // Note, for the 128bit case we don't need to check for overflow, since
    // if the fundamentalAssuptions () hold (checked above), there can't be
    // actual math overflow at this level, there are excess bits to hold the
    // result.  However for the 64bit case it's possible we need to overflow
    // into a 128 bit result.
    //
    if (std::is_same<T, int64_t>::value && (
        (isPositive (v1) && isPositive (v2) && isNegative (result)) ||
        (isNegative (v1) && isNegative (v2) && isPositive (result))
      )
    )
    {
        throw fixed::OverflowException ("Internal 64bit addition overflow");
    }

    return result;
}

template <typename T> T Number::subtraction (const T& v1, const T& v2)
{
    static_assert (
        fundamentalAssumptions (),
        "Logic in subtraction () requires fundamentalAssuptions () hold"
    );

    static_assert (
        std::is_same<T, int64_t>::value || std::is_same<T, __int128_t>::value,
        "Expecting int64_t or int128_t"
    );

    T result = v1 - v2;

    //
    // Note, for the 128bit case we don't need to check for overflow, since
    // if the fundamentalAssuptions () hold (checked above), there can't be
    // actual math overflow at this level, there are extra bits to hold the
    // result.  However for the 64bit case it's possible we need to overflow
    // into a 128 bit result.
    //
    if (std::is_same<T, int64_t>::value && (
        (isPositive (v1) && isNegative (v2) && isNegative (result)) ||
        (isNegative (v1) && isPositive (v2) && isPositive (result))
      )
    )
    {
        throw fixed::OverflowException ("Internal 64bit subtraction overflow");
    }

    return result;
}

Number& Number::operator*= (const Number& rhs)
{
    //
    // We make the extra copy in the event mult throws an exception we don't
    // have to worry about leaving the object in an inconsistent state
    //
    Number number (*this);
    number.mult (rhs);

    *this = number;
    return *this;
}

Number& Number::mult (const Number& rhs)
{
    const unsigned int newDecimalPlaces =
        Precision::getProductDecimalPlaces (
            decimalPlaces (),
            rhs.decimalPlaces (),
            MAX_DECIMAL_PLACES,
            multPrecisionPolicy_
        );

    unsigned int resultingDecimalPlaces;

    if (value64Set () && rhs.value64Set ())
    {
        mult64 (rhs, resultingDecimalPlaces);
    }
    else
    {
        mult128 (rhs, resultingDecimalPlaces);
    }

    //
    // Note, we need to set the true decimal places of the multiplication,
    // setDecimalPlaces () will need to know the current value to be able to
    // correctly adjust to the desired value.  Also note this means that
    // temporarily decimalPlaces_ could be > MAX_DECIMAL_PLACES, which means
    // the object is temporarily in an illegal state, certain operations on the
    // object would be undefined since it would break their assumptions.  This
    // state will be resolved however once setDecimalPlaces () returns, and
    // setDecimalPlaces () itself knows to look out for this.
    //
    decimalPlaces_ = resultingDecimalPlaces;

    //
    // It's possible we had to reduce the precision of the factors in order to
    // be able to fit the result, only setDecimalPlaces to newDecimalPlaces if
    // it's smaller, not valid to increase number of decimal places, as that
    // would imply the result has more precision than we were actually able to
    // provide.  Also note, if we remove this check, it opens us up to another
    // issue we'd need to account for, if we were increasing the decimal places
    // we'd have to do the integerValueOverflowCheck () before doing so, since
    // setDecimalPlaces does not check that increasing the decimal places could
    // cause an overflow, since for all valid Numbers that's not a possibility.
    //
    if (newDecimalPlaces < resultingDecimalPlaces)
    {
        setDecimalPlaces (newDecimalPlaces);
    }

    valueAutoResize ();

    if (integerValueOverflowCheck ())
    {
        throw fixed::OverflowException ("Multiplication caused an overflow");
    }

    return *this;
}

void Number::mult64 (
    const Number& rhs,
    unsigned int& resultingDecimalPlaces
)
{
    assert (value64Set_ && rhs.value64Set_);

    if ((firstBitSet_ (value64_) + firstBitSet_ (rhs.value64_)) >
        FirstBitSet::maxBitPos<int64_t> ())
    {
        Number rhsCopy (rhs);

        upsizeTo128 ();
        rhsCopy.upsizeTo128 ();
        return mult128 (rhsCopy, resultingDecimalPlaces);
    }

    value64_ *= rhs.value64_;
    resultingDecimalPlaces = decimalPlaces () + rhs.decimalPlaces ();
}

void Number::mult128 (
    Number rhs, // note by value
    unsigned int& resultingDecimalPlaces
)
{
    upsizeTo128 ();
    rhs.upsizeTo128 ();

    unsigned int requiredBits =
        firstBitSet_ (value128_) + firstBitSet_ (rhs.value128_);

    if (requiredBits > FirstBitSet::maxBitPos<__int128_t> ())
    {
        multReducePrecision (
            requiredBits - FirstBitSet::maxBitPos<__int128_t> (),
            *this,
            rhs
        );

        value128_ *= rhs.value128_;
        resultingDecimalPlaces = decimalPlaces () + rhs.decimalPlaces ();
    }
    else
    {
        value128_ *= rhs.value128_;
        resultingDecimalPlaces = decimalPlaces () + rhs.decimalPlaces ();
    }
}

void Number::multReducePrecision (
    const unsigned int excessBits,
    Number& n1,
    Number& n2
)
{
    //
    // multReducePrecision should only be called once we've gone to 128bit
    // mode.
    //
    assert (! n1.value64Set () && ! n2.value64Set ());

    //
    // excessBits are the number of bits we need to free up in order to be
    // able to store the result of the multiplication, we convert this
    // number of bits to the number of decimal places we need to reclaim by
    // reducing the precision of the two factors.  We penalize the larger
    // number most as that will affect the precision of the result of the
    // multiplication the least.
    //
    unsigned int dpExcess =
        shiftTable128_.find_if (
            [&] (const ShiftTable<__int128_t>::ShiftValue& sv) {
                return excessBits <= sv.firstBitSet;
            }
        ).decimalPlaces;

    //
    // If we hit this case, the result of the multiplication would be
    // too big for us to store.
    //
    if (dpExcess > (n1.decimalPlaces () + n2.decimalPlaces ()))
    {
        throw fixed::OverflowException ("Multiplicaiton caused an overflow");
    }

    unsigned int n1Idop =
        shiftTable128_.integerDigitsOfPrecision (
            n1.value128_, n1.decimalPlaces ()
        );

    unsigned int n2Idop =
        shiftTable128_.integerDigitsOfPrecision (
            n2.value128_, n2.decimalPlaces ()
        );

    //
    // First prefer to trim trailing zeroes versus dropping actual digits.
    //
    dpExcess -= n1.makeCompact (dpExcess);
    dpExcess -= n2.makeCompact (dpExcess);

    unsigned int n1Dp = n1.decimalPlaces ();
    unsigned int n2Dp = n2.decimalPlaces ();

    if (n1Idop > n2Idop)
    {
        unsigned int dpSaved = std::min (n1Idop - n2Idop, dpExcess);

        n1Dp -= dpSaved;
        dpExcess -= dpSaved;
    }
    else if (n2Idop > n1Idop)
    {
        unsigned int dpSaved = std::min (n2Idop - n1Idop, dpExcess);

        n2Dp -= dpSaved;
        dpExcess -= dpSaved;
    }

    if (dpExcess)
    {
        //
        // If we get here, from above we've made the magnitudes of the
        // two numbers the same, so now we'll remove the same amount
        // from the decimal places of each
        //
        n1Dp -= dpExcess / 2;
        n2Dp -= dpExcess / 2;

        dpExcess = dpExcess & 0x1 ? 1 : 0;

        //
        // If dpExcess was odd, we're still 1 short, penalize the one with
        // the most decimalPlaces at this point.  If each have the same
        // decimal places, still need to have a deterministic test to
        // choose which to penalize, that way n1 * n2 == n2 * n1
        //
        if (dpExcess & 0x1)
        {
            if (n1Dp > n2Dp)
            {
                n1Dp--;
            }
            else if (n2Dp > n1Dp)
            {
                n2Dp--;
            }
            else if (toAbsolute (n1) > toAbsolute (n2))
            {
                n1Dp--;
            }
            else
            {
                n2Dp--;
            }
        }
    }

    n1.setDecimalPlaces (n1Dp);
    n2.setDecimalPlaces (n2Dp);

    //
    // The setDecimalPlaces may have caused a downsize to 64 bits, need to
    // keep their representations at 128...
    //
    n1.upsizeTo128 ();
    n2.upsizeTo128 ();
}

Number& Number::operator/= (const Number& rhs)
{
    //
    // We make the extra copy in the event div throws an exception we don't
    // have to worry about leaving the object in an inconsistent state
    //
    Number number (*this);
    number.div (rhs);

    *this = number;
    return *this;
}

Number& Number::div (const Number& rhs)
{
    if (rhs.isZero ())
    {
        throw fixed::DivideByZeroException ("Division");
    }

    auto quotientDecimalPlaces =
        Precision::getQuotientDecimalPlaces (
            decimalPlaces (),
            rhs.decimalPlaces (),
            MAX_DECIMAL_PLACES,
            divPrecisionPolicy_
        );

    auto requiredDividendShift = quotientDecimalPlaces;

    //
    // This excess shift is with regards to the required shift to achieve
    // the quotientDecimalPlaces
    //
    unsigned int excessDividendShift = 0;

    if (decimalPlaces () < rhs.decimalPlaces ())
    {
        requiredDividendShift += rhs.decimalPlaces () - decimalPlaces ();
    }
    else if (decimalPlaces () > rhs.decimalPlaces ())
    {
        unsigned int dpShift = decimalPlaces () - rhs.decimalPlaces ();

        if (requiredDividendShift >= dpShift)
        {
            requiredDividendShift -= dpShift;
        }
        else
        {
            excessDividendShift = dpShift - requiredDividendShift;
            requiredDividendShift = 0;
        }
    }

    //
    // Want some excess room to be able to do proper rounding.
    //
    if (excessDividendShift < DIVISION_EXTRA_DP_FOR_ROUNDING)
    {
        auto delta = DIVISION_EXTRA_DP_FOR_ROUNDING - excessDividendShift;

        excessDividendShift += delta;
        requiredDividendShift += delta;
    }

    if (value64Set () && rhs.value64Set ())
    {
        div64 (
            rhs,
            quotientDecimalPlaces,
            requiredDividendShift,
            excessDividendShift
        );
    }
    else
    {
        div128 (
            rhs,
            quotientDecimalPlaces,
            requiredDividendShift,
            excessDividendShift
        );
    }

    decimalPlaces_ = quotientDecimalPlaces + excessDividendShift;

    setDecimalPlaces (quotientDecimalPlaces);

    valueAutoResize ();

    if (integerValueOverflowCheck ())
    {
        throw fixed::OverflowException ("Division caused an overflow");
    }

    return *this;
}

void Number::div64 (
    const Number& rhs,
    unsigned int& targetDecimalPlaces,
    unsigned int& requiredDividendShift,
    unsigned int& excessDividendShift
)
{
    const auto rds = requiredDividendShift;

    const auto shiftRoom =
        FirstBitSet::maxBitPos<int64_t> () - firstBitSet_ (value64_);

    const bool need128 =
        (rds > shiftTable64_.MAX_DIGITS) ||
        (shiftRoom < shiftTable64_[rds].firstBitSet);

    if (need128)
    {
        return div128 (
            rhs,
            targetDecimalPlaces,
            requiredDividendShift,
            excessDividendShift
        );
    }

    value64_ = value64_ * shiftTable64_[rds].value / rhs.value64_;
}

void Number::div128 (
    Number rhs, // note, by value
    unsigned int& quotientDecimalPlaces,
    unsigned int& requiredDividendShift,
    unsigned int& excessDividendShift
)
{
    upsizeTo128 ();
    rhs.upsizeTo128 ();

    //
    // Based on the fundamentalAssumptions, no valid combination of dividend
    // and divisor can lead to a requiredDividendShift that would index
    // out of the range of the 128bit shift table.
    //
    assert (requiredDividendShift <= shiftTable128_.MAX_DIGITS);

    const auto shiftRoom =
        FirstBitSet::maxBitPos<__int128_t> () - firstBitSet_ (value128_);

    if (shiftRoom >= shiftTable128_[requiredDividendShift].firstBitSet)
    {
        value128_ =
            value128_ * shiftTable128_[requiredDividendShift].value /
            rhs.value128_;

        return;
    }

    //
    // Strategy now is to shift the divisor as far left as possible, and
    // figure out the number of decimal places we can yield for the quotient
    //
    Number divisor (rhs);

    const auto& sv =
        shiftTable128_.find_if_not (
            [&] (const ShiftTable<__int128_t>::ShiftValue& sv) {
                return shiftRoom >= sv.firstBitSet;
            }
        );

    //
    // It's the shiftValue just before the entry that was found that's needed
    //
    if (sv.decimalPlaces > 0)
    {
        value128_ *= shiftTable128_[sv.decimalPlaces - 1].value;
        requiredDividendShift -= sv.decimalPlaces - 1;
    }

    auto squeezed = squeezeZeros (divisor.value128_);

    excessDividendShift += squeezed;
    divisor.decimalPlaces_ -= squeezed;

    //
    // If we're dividing by a value of one, we don't need the rounding
    // extra space.
    //
    auto roundPlaces =
        absoluteValue<__uint128_t> (divisor.value128_) == 1 ?
            0 : DIVISION_EXTRA_DP_FOR_ROUNDING;

    if (excessDividendShift > roundPlaces)
    {
        auto space = std::min (
            requiredDividendShift,
            excessDividendShift - roundPlaces
        );

        excessDividendShift -= space;
        requiredDividendShift -= space;

        if (! requiredDividendShift)
        {
            value128_ /= divisor.value128_;
            return;
        }
    }

    //
    // At this point we've got the dividend shifted as far left as possible,
    // we'll drop any excessDividendShift that may exist, essentially giving
    // up rounding, however buying us more decimal places of precision for
    // the result.
    //
    auto deltaDps = std::min (requiredDividendShift, excessDividendShift);

    requiredDividendShift -= deltaDps;
    excessDividendShift   -= deltaDps;

    if (! requiredDividendShift)
    {
        value128_ /= divisor.value128_;
        return;
    }

    //
    // At this point we've got the dividend shifted as far left as possible,
    // there is no excess dividend shift left, we need to start reducing the
    // precision of the quotient.
    //
    deltaDps = std::min (quotientDecimalPlaces, requiredDividendShift);

    quotientDecimalPlaces -= deltaDps;
    requiredDividendShift -= deltaDps;

    if (! requiredDividendShift)
    {
        value128_ /= divisor.value128_;
        return;
    }

    //
    // Note, instead of depleating the quotientDecimalPlaces completely,
    // another option would be to reduce the decimal places of the divisor,
    // e.g. if it has 14 decimalPlaces we could trim a few off the end, however
    // that can affect the result quite dramatically when dividing very large
    // numbers by very small numbers, it was deemed more correct to lose all
    // the precision in the form of the provided decimal places in the
    // quotient.  If the client isn't happy with the precision of the result
    // they can modify the precision of the divisors themselves so it's
    // explicit.
    //

    throw fixed::OverflowException (
        "Division quotient would be too large"
    );
}

Number& Number::operator%= (const Number& rhs)
{
    //
    // We make the extra copy in the event div throws an exception we don't
    // have to worry about leaving the object in an inconsistent state
    //
    Number number (*this);
    number.remainder (rhs);

    *this = number;
    return *this;
}

Number& Number::remainder (const Number& rhs)
{
    if (rhs.isZero ())
    {
        throw fixed::DivideByZeroException ("Remainder divide by zero");
    }

    if (decimalPlaces () == rhs.decimalPlaces ())
    {
        remainderEqualDecimalPlaces (rhs);
    }
    else
    {
        auto newDecimalPlaces =
            std::max (
                decimalPlaces (),
                rhs.decimalPlaces ()
            );

        if (decimalPlaces () == newDecimalPlaces)
        {
            Number rhsCopy = rhs;
            rhsCopy.setDecimalPlaces (newDecimalPlaces);

            remainderEqualDecimalPlaces (rhsCopy);
        }
        else
        {
            setDecimalPlaces (newDecimalPlaces);
            remainderEqualDecimalPlaces (rhs);
        }
    }

    return *this;
}

void Number::remainderEqualDecimalPlaces (const Number& rhs) noexcept
{
    if (value64Set () && rhs.value64Set ())
    {
        value64_ %= rhs.value64_;
    }
    else
    {
        upsizeTo128 ();

        value128_ %= rhs.value64Set () ?
                         static_cast<__int128_t> (rhs.value64_) :
                         rhs.value128_;

        valueAutoResize ();
    }
}

bool Number::relationalOperation (
    const Number& lhs,
    const Number& rhs,
    RelationalOperation64 relop64,
    RelationalOperation128 relop128
) noexcept
{
    bool retval = false;

    if (lhs.decimalPlaces () == rhs.decimalPlaces ())
    {
        retval = relationalValuesCmp (lhs, rhs, relop64, relop128);
    }
    else if (lhs.decimalPlaces () > rhs.decimalPlaces ())
    {
        Number rhsCopy (rhs);
        rhsCopy.setDecimalPlaces (lhs.decimalPlaces ());
        retval = relationalValuesCmp (lhs, rhsCopy, relop64, relop128);
    }
    else
    {
        Number lhsCopy (lhs);
        lhsCopy.setDecimalPlaces (rhs.decimalPlaces ());
        retval = relationalValuesCmp (lhsCopy, rhs, relop64, relop128);
    }

    return retval;
}

bool operator< (const Number& lhs, const Number& rhs)
{
    return Number::relationalOperation (
        lhs,
        rhs,
        [] (const int64_t& v1, const int64_t& v2) { return v1 < v2; },
        [] (const __int128_t& v1, const __int128_t& v2) { return v1 < v2; }
    );
}

bool operator<= (const Number& lhs, const Number& rhs)
{
    return Number::relationalOperation (
        lhs,
        rhs,
        [] (const int64_t& v1, const int64_t& v2) { return v1 <= v2; },
        [] (const __int128_t& v1, const __int128_t& v2) { return v1 <= v2; }
    );
}

bool operator> (const Number& lhs, const Number& rhs)
{
    return ! (lhs <= rhs);
}

bool operator>= (const Number& lhs, const Number& rhs)
{
    return ! (lhs < rhs);
}

bool Number::relationalValuesCmp (
    const Number& lhs,
    const Number& rhs,
    RelationalOperation64 relop64,
    RelationalOperation128 relop128
) noexcept
{
    if (lhs.value64Set_ && rhs.value64Set_)
    {
        return relop64 (lhs.value64_, rhs.value64_);
    }
    else if (! lhs.value64Set_ && ! rhs.value64Set_)
    {
        return relop128 (lhs.value128_, rhs.value128_);
    }
    else if (lhs.value64Set_)
    {
        return relop128 (static_cast<__int128_t> (lhs.value64_), rhs.value128_);
    }

    return relop128 (lhs.value128_, static_cast<__int128_t> (rhs.value64_));
}

bool operator== (const Number& lhs, const Number& rhs)
{
    return Number::relationalOperation (
        rhs,
        lhs,
        [] (const int64_t& v1, const int64_t& v2) { return v1 == v2; },
        [] (const __int128_t& v1, const __int128_t& v2) { return v1 == v2; }
    );
}

bool operator!= (const Number& lhs, const Number& rhs)
{
    return ! (lhs == rhs);
}

const Number operator+ (const Number& lhs, const Number& rhs)
{
    Number number (lhs);

    return number.addSub (
        rhs,
        Number::addition<int64_t>,
        Number::addition<__int128_t>
    );
}

const Number operator- (const Number& lhs, const Number& rhs)
{
    Number number (lhs);

    return number.addSub (
        rhs,
        Number::subtraction<int64_t>,
        Number::subtraction<__int128_t>
    );
}

const Number operator* (const Number& lhs, const Number& rhs)
{
    Number number (lhs);

    return number.mult (rhs);
}

const Number operator/ (const Number& lhs, const Number& rhs)
{
    Number number (lhs);

    return number.div (rhs);
}

const Number operator% (const Number& lhs, const Number& rhs)
{
    Number number (lhs);

    return number.remainder (rhs);
}

//
// Utility function meant for Number::Number (str)
//
unsigned long long Number::convertStrToVal (
    const char* cptr,
    char*& endptr,
    Sign& sign,
    const std::string& errMsgHeader
)
{
    if (cptr[0] == '\0')
    {
        throw fixed::BadValueException (errMsgHeader + " empty str");
    }

    unsigned long long value;

    sign = Sign::POSITIVE;

    if (cptr[0] == '+')
    {
        ++cptr;
    }

    if (cptr[0] == '-')
    {
        sign = Sign::NEGATIVE;
        ++cptr;
    }

    if (! isdigit (cptr[0]))
    {
        throw fixed::BadValueException (
            errMsgHeader + "value does not start with a digit"
        );
    }

    errno = 0;

    value = std::strtoull (cptr, &endptr, 10);

    if (errno == ERANGE || errno == EINVAL)
    {
        throw fixed::BadValueException (
            errMsgHeader + "bad integer value, may be too large."
        );
    }

    return value;
}

bool Number::integerValueOverflowCheck ()
{
    //
    // From the fundamental assumptions, we don't have to check for integer
    // overflow if we're in 64bit land, since if the value fits in 64 bits
    // we can't have an integer overflow.  The assumption is that
    // MAX_INTEGER_VAL >= int64::max.  There is a corner case where if
    // MAX_INTEGER_VAL == int64::max, and decimalPlaces is 0, then the
    // value int64::min would fit in an int64 and still be an overflow,
    // however valueAutoResize checks for this and stores values of
    // int64::min in a value128_.  The integerOverflowCheckVal values have
    // been seeded for each decimal place in the table based on MAX_INTEGER_VAL
    //
    static_assert (
        fundamentalAssumptions (),
        "Number::integerValueOverflowCheck assumes fundamentalAssumptions() "
        "hold"
    );

    if (value64Set_)
    {
        return false;
    }

    return integerValueOverflowCheck (value128_, decimalPlaces ());
}

bool Number::integerValueOverflowCheck (
    __int128_t value,
    unsigned int decimalPlaces
)
{
    const auto& sval = shiftTable128_[decimalPlaces];

    return isNegative (value) ?
            value < sval.integerOverflowCheckValNeg :
            value > sval.integerOverflowCheckValPos;
}

} // namespace fixed
