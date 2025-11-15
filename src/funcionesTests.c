#include <stdio.h>

long get_int() {
    long x;
    scanf("%ld", &x);
    return x;
}

void print_int(long x) {
    printf("%ld\n", x);
}

void mostrar(long x) {
    printf("valor: %ld\n", x);
}
