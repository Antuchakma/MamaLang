#include <stdio.h>
#include <stdlib.h>

int main(void) {
    long long a = 20;
    long long b = 10;
    if ((a > b)) {
    printf("%s\n", "A boro");
    }
    else {
    printf("%s\n", "B boro");
    }
    if ((a > 15)) {
    if ((b < 20)) {
    printf("%s\n", "Nested");
    }
    }
    while ((a > 15)) {
    printf("%lld\n", (long long)(a));
    a = (a - 1);
    }
    return 0;
}
