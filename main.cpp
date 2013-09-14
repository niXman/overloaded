
#include <iostream>
#include <type_traits>
#include <cassert>

#include "overloaded_function.hpp"

/***************************************************************************/

void f1() {std::cout << __PRETTY_FUNCTION__ << std::endl;}
int f2(const int&) {std::cout << __PRETTY_FUNCTION__ << std::endl;return 33;}
void f3(char const*) {std::cout << __PRETTY_FUNCTION__ << std::endl;}
void f4(int, double) {std::cout << __PRETTY_FUNCTION__ << std::endl;}

template<typename T>
void foo(T) {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}
template<typename T>
void foo() {
	std::cout << __PRETTY_FUNCTION__ << std::endl;
}

/***************************************************************************/

#define TEST(expected, ...) \
	std::cout << "test: \"" << #__VA_ARGS__ << "\"... " \
		<< ((expected == (__VA_ARGS__)) ? "done" : "error!") << std::endl;

/***************************************************************************/

int main() {
	namespace bof = boost::overloaded_function;

	auto map1 = bof::create(f1, f2, f3, f4);
	TEST(true, bof::calls(map1) == 4);
	TEST(true, bof::exists<void()>(map1));
	TEST(true, bof::exists(map1, f1));
	auto map2 = bof::erase(map1, f1);
	TEST(true, bof::calls(map2) == 3);
	TEST(false, bof::exists(map2, f1));
	auto map3 = bof::insert(map2, f1);
	bof::invoke(map3);
	TEST(true, bof::exists<void()>(map3));
	TEST(true, bof::exists(map3, f1));

	auto func = bof::make_overloaded_function(f2, f4);
	int r1 = func(2);
	func(3, 2.3d);

//std::cout << func::invoke(map, 2);
//	invoke(map, "3");
//	invoke(map);
//	double d = 2.33;
//	const double *dp = &d;
//	invoke(map, 3, dp);
}

/***************************************************************************/
