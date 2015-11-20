#include <stdio.h>

int fac_times (int n, int acc) {
    if (n == 0) return acc;
    else return fac_times(n - 1, acc * n);
}

int fact(int n)
{
  return n <= 1 ? 1 : n * fact(n-1);
}

int foo(int a, int b) {
    if (a && b)
        return foo(a - 1, b - 1);
    return a + b;
}

int main() {
 foo(5, 5);
 return 0;
}

