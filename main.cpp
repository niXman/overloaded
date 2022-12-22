
#undef NDEBUG

#include <iostream>
#include <cassert>

#include "overloaded_function.hpp"

/***************************************************************************/

int f1_counter = 0;
__attribute__ ((noinline)) // for tests only
void f1() {std::cout << __PRETTY_FUNCTION__ << std::endl; f1_counter++;}

int f2_counter = 0;
__attribute__ ((noinline))
int f2(const int &v) {std::cout << __PRETTY_FUNCTION__ << std::endl; f2_counter++; return v*2;}

int f3_counter = 0;
__attribute__ ((noinline))
int f3(int &v) {std::cout << __PRETTY_FUNCTION__ << std::endl; f3_counter++; v = v*2; return v;}

int f4_counter = 0;
__attribute__ ((noinline))
int f4(int v) {std::cout << __PRETTY_FUNCTION__ << std::endl; f4_counter++; return v+2;}

struct callable {
    callable(const callable &) = delete;
    callable& operator= (const callable &) = delete;

    callable(callable &&) = default;
    callable& operator= (callable &&) = default;

    int operator()(int l, int r) const { return l+r;}
};

/***************************************************************************/

#define RT_TEST(expected, ...) \
    std::cout << "rt test: \"" << #__VA_ARGS__ " == " #expected "\"..." \
        << ((expected == (__VA_ARGS__)) ? "done" : "error!") << std::endl; \
    assert(expected == ((__VA_ARGS__)));

#define CT_TEST(expected, ...) \
    static_assert(expected == (__VA_ARGS__), #__VA_ARGS__ " == " #expected); \
    std::cout << "ct test: \"" << #__VA_ARGS__ << "\"... " \
        << ((expected == (__VA_ARGS__)) ? "done" : "error!") << std::endl;

/***************************************************************************/

int main() {
    { // func pointer
        auto *fp = &f2;
        auto o = overloaded::make(fp);

        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(const int &)>());
        CT_TEST(false, o.exists<const char* ()>());

        RT_TEST(0, f2_counter);
        auto r = o.invoke(4);
        RT_TEST(8, r);
        RT_TEST(1, f2_counter);
        f2_counter = 0;
    }
    { // func
        auto o = overloaded::make(f1);

        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<void()>());
        CT_TEST(true, o.exists<void(void)>());

        RT_TEST(0, f1_counter);
        o();
        RT_TEST(1, f1_counter);
        f1_counter = 0;
    }
    { // many funcs
        auto o = overloaded::make(f1, f2);
        CT_TEST(true, o.size() == 2);
        CT_TEST(true, o.exists<void(void)>());
        CT_TEST(true, o.exists<int(const int &)>());
        CT_TEST(false, o.exists<int(const double &)>());

        RT_TEST(0, f1_counter);
        o();
        RT_TEST(1, f1_counter);
        f1_counter = 0;

        RT_TEST(0, f2_counter);
        auto r = o.invoke(2);
        RT_TEST(4, r);
        RT_TEST(1, f2_counter);
        f2_counter = 0;
    }
    { // lambda r-value
        auto counter = 0;
        auto o = overloaded::make([&counter](int v){ counter++; return v*2;});
        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(int)>());
        CT_TEST(false, o.exists<void(void)>());

        RT_TEST(0, counter);
        auto r = o(3);
        RT_TEST(6, r);
        RT_TEST(1, counter);
    }
    { // lambda l-value
        auto counter = 0;
        auto l = [&counter](int v){ counter++; return v*2;};
        auto o = overloaded::make(l);
        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(int)>());
        CT_TEST(false, o.exists<void(void)>());

        RT_TEST(0, counter);
        auto r = o(3);
        RT_TEST(6, r);
        RT_TEST(1, counter);
    }
    { // std::function
        auto counter = 0;
        std::function<int(int)> f = [&counter](int v){ counter++; return v*2;};
        auto o = overloaded::make(f);
        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(int)>());
        CT_TEST(false, o.exists<void(void)>());

        RT_TEST(0, counter);
        auto r = o(3);
        RT_TEST(6, r);
        RT_TEST(1, counter);
    }
    { // any non-copyable but movable type r-value
        auto o = overloaded::make(callable{});
        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(int, int)>());
        CT_TEST(false, o.exists<void(void)>());

        auto r = o(3, 3);
        RT_TEST(6, r);
    }
    { // for const l-value ref
        auto o = overloaded::make(f2);

        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(const int &)>());

        RT_TEST(0, f2_counter);
        int v = 3;
        auto r = o(v);
        RT_TEST(6, r);
        RT_TEST(1, f2_counter);
        f2_counter = 0;
    }
    { // for non-const l-value ref
        auto o = overloaded::make(f3);

        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(int &)>());

        RT_TEST(0, f3_counter);
        int v = 3;
        auto r = o(v);
        RT_TEST(6, v);
        RT_TEST(6, r);
        RT_TEST(1, f3_counter);
        f3_counter = 0;
    }
    { // for non-const l-value ref
        auto o = overloaded::make(f4);

        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(int)>());

        int v = 3;
        auto r = o.invoke(v);
        RT_TEST(3, v);
        RT_TEST(5, r);
        RT_TEST(1, f4_counter);
        f4_counter = 0;
    }
    { // ref to any non-copyable but movable type
        callable c{};
        auto o = overloaded::make(c);

        CT_TEST(true, o.size() == 1);
        CT_TEST(true, o.exists<int(int, int)>());
        CT_TEST(false, o.exists<void(void)>());

        auto r = o(3, 3);
        RT_TEST(6, r);
    }

    return EXIT_SUCCESS;
}

/***************************************************************************/
