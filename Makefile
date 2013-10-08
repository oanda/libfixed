#
# The MIT License (MIT)
#
#
# Copyright (c) 2013 OANDA Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#

BUILD_OUTDIR ?= ./build

LIB_INCS := -I./include -I./src

TEST_INCS := $(LIB_INCS) -I./test

LIB_SRC := \
    src/Number.cpp \
    src/Precision.cpp \
    src/Rounding.cpp

TEST_SRC := \
    test/FirstBitSetTests.cpp \
    test/NumberArithmeticTests.cpp \
    test/NumberIntConstructorFailTests.cpp \
    test/NumberIntConstructorTests.cpp \
    test/NumberFpConstructorFailTests.cpp \
    test/NumberFpConstructorTests.cpp \
    test/NumberRelationalTests.cpp \
    test/NumberToFpTests.cpp \
    test/RoundingTests.cpp \
    test/SqueezeZerosTests.cpp \
    test/UnitTest.cpp

LIB_OBJ := $(patsubst src/%,$(BUILD_OUTDIR)/%,$(LIB_SRC:.cpp=.o))

TEST_OBJ := $(patsubst test/%,$(BUILD_OUTDIR)/%,$(TEST_SRC:.cpp=.o))

CXX ?= g++

WARN_FLAGS ?= -Wall -Wextra -Werror
DEBUG_FLAGS ?= -ggdb
OPTIMIZATION_FLAGS ?= -O3
ARCH_FLAGS ?= -m64
#OPTIMIZATION_FLAGS := -O0 -ftest-coverage -fprofile-arcs

CXXFLAGS := -std=c++11 -U__STRICT_ANSI__ $(WARN_FLAGS) $(DEBUG_FLAGS) $(OPTIMIZATION_FLAGS) $(ARCH_FLAGS) $(INCS)

ARFLAGS ?= crv

all: lib test

lib: $(BUILD_OUTDIR)/libfixed.a

test: $(BUILD_OUTDIR)/fixed_unit_tests
	$(BUILD_OUTDIR)/fixed_unit_tests

$(LIB_OBJ): $(BUILD_OUTDIR)

$(TEST_OBJ): $(BUILD_OUTDIR)

$(BUILD_OUTDIR):
	@[ -d $(BUILD_OUTDIR) ] || mkdir -p $(BUILD_OUTDIR)

$(BUILD_OUTDIR)/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) $(LIB_INCS) -c -o $@ $<

$(BUILD_OUTDIR)/%.o : test/%.cpp
	$(CXX) $(CXXFLAGS) $(TEST_INCS) -c -o $@ $<

$(BUILD_OUTDIR)/libfixed.a: $(LIB_OBJ)
	$(AR) $(ARFLAGS) $@ $^

$(BUILD_OUTDIR)/fixed_unit_tests: $(TEST_OBJ) $(BUILD_OUTDIR)/libfixed.a
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_OUTDIR)
