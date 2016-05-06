#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <cmath>
#include <array>
#include <assert.h>

typedef int64_t TINT;

typedef std::array<TINT, 2> TComplex;

//=================================================================
void WaitForEnter ()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================
bool IsPrime (TINT n)
{
    if (n < 2)
        return false;

    TINT sqrtn = (TINT)std::sqrt(n);

    for (TINT i = 2; i <= sqrtn; ++i)
    {
        if (n % i == 0)
            return false;
    }

    return true;
}

//=================================================================
bool HasImaginary (TINT n, std::array<TINT, 2> &imaginaries)
{
    for (TINT i = 0; i < n; ++i)
    {
        if (((i*i) % n) + 1 == n)
        {
            imaginaries[0] = i;
            imaginaries[1] = n - i;
            return true;
        }
    }
    return false;
}

//=================================================================
TINT FindPrimeWithImaginary (TINT minimum, std::array<TINT, 2> &imaginaries)
{
    while (minimum >= 0)
    {
        if (IsPrime(minimum) && HasImaginary(minimum, imaginaries))
            return minimum;
        ++minimum;
    }
    assert(false);
    return 0;
}

//=================================================================
int main (int argc, char **argv)
{
    // define the complex numbers to multiply
    TComplex A = { 1, 1 };
    TComplex B = { 0, 1 };

    // find a prime with values for i, and store the prime and those values
    std::array<TINT, 2> imaginaries;
    TINT prime = FindPrimeWithImaginary(100, imaginaries);

    // convert A and B to parallelized values
    TINT valueA0 = (A[0] + A[1] * imaginaries[0]) % prime;
    TINT valueA1 = (A[0] + A[1] * imaginaries[1]) % prime;
    TINT valueB0 = (B[0] + B[1] * imaginaries[0]) % prime;
    TINT valueB1 = (B[0] + B[1] * imaginaries[1]) % prime;

    // do the operation
    TINT value0 = (valueA0 * valueB0) % prime;
    TINT value1 = (valueA1 * valueB1) % prime;

    // decode results
    //TINT a = 

    int ijkl = 0;

    //ProcessNumbers<0, 50000>(IsAnyNumber, HasImaginary<2>);

    WaitForEnter();
    return 0;
}

/*
TODO:
* make this work
* generalize to hypercomplex?

THOUGHTS:
* maybe we can put both values into one value?
* what size does a prime have to be to support a given operation?
? which parts of the paper are novel?

*/