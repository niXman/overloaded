overloaded_function
=========

This library allows to overload different functions into a single function object without any run-time overhead.

For the following example:
```cpp
/***************************************************************************/

__attribute__ ((noinline))
int f2(const int &v) {return v*2;}

__attribute__ ((noinline))
int f3(int &v) {v = v*2; return v;}

/***************************************************************************/

int main(int argc, char **) {
    int res = 0;
    auto *fp = &f2;
    auto o0 = overloaded::make(fp);
    res += o0(argc);

    auto o1 = overloaded::make(f3);
    res += o1(argc);

    return res;
}

/***************************************************************************/
```
the following ASM will be generated:
```asm
f2(int const&):
        mov     eax, DWORD PTR [rdi]
        add     eax, eax
        ret
f3(int&):
        mov     eax, DWORD PTR [rdi]
        add     eax, eax
        mov     DWORD PTR [rdi], eax
        ret
main:
        sub     rsp, 8
        mov     DWORD PTR [rsp+4], edi
        lea     rdi, [rsp+4]
        call    f2(int const&)
        mov     edx, eax
        call    f3(int&)
        add     rsp, 8
        add     eax, edx
        ret
```
As far as you can see, there is nothing superfluous!


Example
=========
```cpp
int add(int a, int b) { return a+b; }
double mul(double a, double b){ return a*b; }

auto func = overloaded::make(add, mul);

std::assert(func(2, 2) == 4);
std::assert(func(2.2, 2.2) == 4.84);
```
