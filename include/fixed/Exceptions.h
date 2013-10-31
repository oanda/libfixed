//
// The MIT License (MIT)
//
//
// Copyright (c) 2013 OANDA Corporation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//

#ifndef FIXED_EXCEPTIONS_H
#define FIXED_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace fixed {

class Exception : public std::runtime_error
{
  public:
    Exception (const std::string& msg)
      : std::runtime_error ("fixed::Exception::" + msg)
    {}

    virtual ~Exception () noexcept {}
};

class OverflowException : public fixed::Exception
{
  public:
    OverflowException (const std::string& msg)
      : Exception ("Overflow: " + msg)
    {}

    virtual ~OverflowException () noexcept {}
};

class DivideByZeroException : public fixed::Exception
{
  public:
    DivideByZeroException (const std::string& msg)
      : Exception ("DivideByZero: " + msg)
    {}

    virtual ~DivideByZeroException () noexcept {}
};

class BadValueException : public fixed::Exception
{
  public:
    BadValueException (const std::string& msg)
      : Exception ("BadValue: " + msg)
    {}

    virtual ~BadValueException () noexcept {}
};

} // namespace fixed

#endif // FIXED_EXCEPTIONS_H
