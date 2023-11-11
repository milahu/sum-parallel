// https://stackoverflow.com/a/47162173/10440128

#include <stdio.h>
int main(int argc, char** argv) {
    long sum = 0;
    long i = 0;
    while(scanf("%ld", &i) == 1) {
        sum = sum + i;
    }
    printf("%ld\n", sum);
    return 0;
}
