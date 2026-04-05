#include <stdio.h>
#include <stdlib.h>

/* Forward declarations */
long long square(long long n);
long long max(long long a, long long b);

long long square(long long n) {
{
    return (n * n);
    }
}

long long max(long long a, long long b) {
{
    if ((a > b)) {
    return a;
    }
    return b;
    }
}


int main(void) {
    long long result = square(7);
    printf("%s\n", "7 squared =");
    printf("%lld\n", (long long)(result));
    long long bigger = max(15, 42);
    printf("%s\n", "Max of 15 and 42 =");
    printf("%lld\n", (long long)(bigger));
    long long val = max(square(3), square(2));
    printf("%s\n", "Max of 9 and 4 =");
    printf("%lld\n", (long long)(val));
    return 0;
}
