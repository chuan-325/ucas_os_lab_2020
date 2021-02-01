#include <stdlib.h>

int x;

void srand(unsigned seed)
{
    x = seed;
}

int rand()
{
    long long tmp = 0x5deece66d * x + 0xb;
    x = tmp & 0x7fffffff;
    return x;
}
