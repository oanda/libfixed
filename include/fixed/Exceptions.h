#ifndef FIXED_EXCEPTIONS_H
#define FIXED_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace fixed {

class OverflowException : public std::runtime_error
{
  public:
    OverflowException (const std::string& msg)
      : std::runtime_error ("OverflowException: " + msg)
    {}

    virtual ~OverflowException () noexcept {}
};

class DivideByZeroException : public std::runtime_error
{
  public:
    DivideByZeroException (const std::string& msg)
      : std::runtime_error ("DivideByZeroException: " + msg)
    {}

    virtual ~DivideByZeroException () noexcept {}
};

class BadValueException : public std::runtime_error
{
  public:
    BadValueException (const std::string& msg)
      : std::runtime_error ("BadValueException: " + msg)
    {}

    virtual ~BadValueException () noexcept {}
};

} // namespace fixed

#endif // FIXED_EXCEPTIONS_H
