overloaded_function
=========

This library allows to overload different functions into a single function object.


example
=========
```cpp

int gv = 0;

void f1(int v) {gv=v;}
int  f2() {return gv;}
void f3(int v) {gv=v*3;}

// ...

using namespace calls_map;

// create initialized fusion::map object
auto map = func::create(f1, f2);

// check if function with signature 'int()' exists
if ( func::exists<int()>(map) )
  // call it without args
  std::cout << func::invoke(map) << std::endl; // 0

// check if function with signature 'void(int)' exists
if ( func::exists<void(int)>(map) )
  // call it and pass 33
  func::invoke(map, 33);

if ( func::exists<int()>(map) )
  std::cout << func::invoke(map) << std::endl; // 33

// ...

// check if function with signature 'void(int)' and address of 'f3()' exists
if ( ! func::exists(map, f3) ) {
  // insert 'f3()' and call it
  auto map2 = func::insert(map, f3);
  // call it and pass 4
  func::invoke(map2, 4);
  
  std::cout << func::invoke(map2) << std::endl; // 8
}

// ...

auto map2 = func::insert(map, [](const char *s){std::cout << s << std::endl;});
if ( func::exists<void(const char*)>(map) )
  func::invoke(map2, "Hello!");

```
