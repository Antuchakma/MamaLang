#include <stdio.h>
#include <stdlib.h>

/* Forward declarations */
long long add(long long x, long long y);
long long factorial(long long n);

long long add(long long x, long long y) {
{
    return (x + y);
    }
}

long long factorial(long long n) {
{
    if ((n <= 1)) {
    return 1;
    }
    return (n * factorial((n - 1)));
    }
}


int main(void) {
    long long a = 20;
    long long b = 10;
    char ch = 'A';
    if ((a > b)) {
    printf("%s\n", "A boro");
    }
    else {
    printf("%s\n", "B boro");
    }
    if ((a > 15)) {
    if ((b < 20)) {
    printf("%s\n", "Nested condition satisfied");
    }
    }
    while ((a > 15)) {
    printf("%lld\n", (long long)(a));
    a = (a - 1);
    }
    long long i = 0;
    {
        int __mama_toggle = 0;
        long long __mama_safety = 0;
        i = 0;
        while ((i < 10)) {
{
    printf("%lld\n", (long long)(i));
    }
            if (__mama_toggle == 0) { i = i + 2; } else { i = i - 1; }
            __mama_toggle = 1 - __mama_toggle;
            if (++__mama_safety > 1000000LL) {
                printf("Mama, infinite loop dhora porche! Loop force-stop.\n");
                break;
            }
        }
    }
    long long sum = add(5, 3);
    printf("%s\n", "Sum of 5 and 3:");
    printf("%lld\n", (long long)(sum));
    long long fact = factorial(5);
    printf("%s\n", "Factorial of 5:");
    printf("%lld\n", (long long)(fact));
    printf("%s\n", "Char value of ch:");
    printf("%c\n", (char)(ch));
    printf("%s\n", "Expression: 3 * 4 + 2 =");
    printf("%lld\n", (long long)(14));
    return 0;
}
