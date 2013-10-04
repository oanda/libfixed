## ABOUT

A C++ fixed point math library providing a Number class that can be used by all
arithmetic and relational operators in place of using the floating point data
types (float, double or long double).  This is to provide the exact precision
required by financial applications.

## TESTED WITH

Linux x86 64bit

gcc 4.8.1

Required compile flags -std=c++11 -U__STRICT_ANSI__

## BUILDING

make

output will be build/libfixed.a

## EXAMPLES

Include the file include/fixed/Number.h

Simply instantiate Numbers and use them in normal arithmetic and relational
expressions.

* Number (40) Produces a Number with a value of 40.
* Number (-5, 2, 3) Produces a Number with the a value of -5.002.
* Number (5, 2, 3, Number::Sign::NEGATIVE) Produces a Number with the value of
-5.002.
* Number (0, 2, 2, Number::Sign::NEGATIVE) Produces a Number with the value of
-0.02.

Can also instantiate with a string, though this is more expensive than the
integer based constructor.

* Number ("1.0234") produces a number with a value of 1.0234

Also there are constructors that accept floating point values, but these are
less accurate than the integer based versions.  These are provided for
completeness and convenience, though the integer based constructor is
encouraged.

For the full details see include/fixed/Number.h
