#pragma once

#------------------------------------------------------------------------------
# MIT License
#
# Copyright (c) 2020 Bob Hood
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
#------------------------------------------------------------------------------

#include <vector>
#include <string>
#include <map>
#include <tuple>
#include <cmath>

#define M_PI 3.14159265

using StringList = std::vector<std::string>;
using StringListIter = StringList::iterator;
using StringListConstIter = StringList::const_iterator;

using ListStringList = std::vector<StringList>;
using ListStringListIter = ListStringList::iterator;
using ListStringListConstIter = ListStringList::const_iterator;

#if 0
template <typename T = int, typename U = int>
std::vector<T>&& range(T start, T stop, U step = 1)
{
    std::vector<T> v;
    do
    {
        v.push_back(start);
        start += step;
    } while (start < (stop + 1));
    return std::move(v);
}
#endif

template <typename T>
bool compare_f(T lhs, T rhs, T epsilon = static_cast<T>(0.005))
{
    return std::fabs(lhs - rhs) < epsilon;
}
