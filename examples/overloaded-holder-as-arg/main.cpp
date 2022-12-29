
// The MIT License (MIT)
//
// Copyright (c) 2013-2023 niXman (github dot nixman at pm.me)
//
// This file is the part of the project 'overloaded_function':
//       github.com/nixman/overloaded_function
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// this example demonstrates the abbility to use `overloaded` for callable types
// with identical signature.

#include <iostream>
#include <cassert>

#include <overloaded_function.hpp>

/***************************************************************************/
// the functions which we plan to call

struct tag_add{};
struct tag_sub{};

int add(tag_add, int l, int r) { return l+r; }

int sub(tag_sub, int l, int r) { return l-r; }

/***************************************************************************/
// describing the `overloaded` holder

using holder_type = typename overloaded::make_overloaded<
     int(tag_add, int, int)
    ,int(tag_sub, int, int)
>::type;

/***************************************************************************/
// the class which we plan will use a `overloaded` holder

struct myclass {
    myclass(holder_type &&h)
        :m_holder{std::move(h)}
    {}

    int add(int l, int r) const noexcept { return m_holder(tag_add{}, l, r); }
    int sub(int l, int r) const noexcept { return m_holder(tag_sub{}, l, r); }

private:
    holder_type m_holder;
};

/***************************************************************************/

int main() {
    // init the holder
    holder_type h = overloaded::make(&add, &sub);

    // constructing the user class
    myclass cls{std::move(h)};

    // usage
    int a = cls.add(2, 2);
    int b = cls.sub(a, 2);

    // test for correctness
    assert(b == 2);

    return EXIT_SUCCESS;
}

/***************************************************************************/
