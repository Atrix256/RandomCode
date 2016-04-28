#include <stdio.h>

void CalculateMask(unsigned int mask, unsigned int &a, unsigned int &b, unsigned int &highest)
{
    a = 1;
    b = 1;

    for (unsigned int i = 0; i < 8; ++i)
    {
        unsigned int bit = 1 << i;

        if (mask & bit)
            a *= (i+1);
        else
            b *= (i+1);
    }

    highest = a > b ? a : b;
}

int main(int argc, char** argv)
{
    unsigned int bestMask = 0;
    unsigned int bestValue = 0;

    for (unsigned int i = 0; i < 128; ++i) {
        unsigned int a, b, highest;

        CalculateMask(i, a, b, highest);

        if (i == 0 || highest < bestValue) {
            bestMask = i;
            bestValue = highest;
        }
    }

    printf("best mask = %u, with a value of %u\r\n", bestMask, bestValue);

    unsigned int a, b, highest;
    CalculateMask(bestMask, a, b, highest);

    printf("A = [");
    for (unsigned int i = 0; i < 8; ++i) {

        unsigned int bit = 1 << i;

        if (bestMask & bit)
            printf("%u,", i+1);
    }
    printf("%c] = %u\r\n", 8, a);

    printf("B = [");
    for (unsigned int i = 0; i < 8; ++i) {

        unsigned int bit = 1 << i;

        if ((bestMask & bit) == 0)
            printf("%u,", i+1);
    }
    printf("%c] = %u\r\n", 8, b);

    return 0;
}

