#include <stdio.h>
#include <algorithm>
#include <array>

// x = a mod m
struct SEquation
{
    int a;
    int m;
};
 
//=================================================================================
unsigned int ExtendedEuclidianAlgorithm (int smaller, int larger, int &s, int &t)
{
    // make sure A <= B before starting
    bool swapped = false;
    if (larger < smaller)
    {
        swapped = true;
        std::swap(smaller, larger);
    }
 
    // set up our storage for the loop.  We only need the last two values so will
    // just use a 2 entry circular buffer for each data item
    std::array<int, 2> remainders = { larger, smaller };
    std::array<int, 2> ss = { 1, 0 };
    std::array<int, 2> ts = { 0, 1 };
    int indexNeg2 = 0;
    int indexNeg1 = 1;
 
    // loop
    while (1)
    {
        // calculate our new quotient and remainder
        int newQuotient = remainders[indexNeg2] / remainders[indexNeg1];
        int newRemainder = remainders[indexNeg2] - newQuotient * remainders[indexNeg1];
 
        // if our remainder is zero we are done.
        if (newRemainder == 0)
        {
            // return our s and t values as well as the quotient as the GCD
            s = ss[indexNeg1];
            t = ts[indexNeg1];
            if (swapped)
                std::swap(s, t);
            return remainders[indexNeg1];
        }
 
        // calculate this round's s and t
        int newS = ss[indexNeg2] - newQuotient * ss[indexNeg1];
        int newT = ts[indexNeg2] - newQuotient * ts[indexNeg1];
 
        // store our values for the next iteration
        remainders[indexNeg2] = newRemainder;
        ss[indexNeg2] = newS;
        ts[indexNeg2] = newT;
 
        // move to the next iteration
        std::swap(indexNeg1, indexNeg2);
    }
}
 
//=================================================================================
void WaitForEnter ()
{
    printf("\nPress Enter to quit");
    fflush(stdin);
    getchar();
}
 
//=================================================================================
int main(int argc, char **argv)
{
    const SEquation equations[] =
    {
        { 2, 3 },
        { 2, 4 },
        { 1, 5 }
    };

    const int c_numEquations = sizeof(equations) / sizeof(equations[0]);

    // print out the equations
    printf("Solving for x:\n");
    for (int i = 0; i < c_numEquations; ++i)
        printf("eq %i:  x = %i mod %i\n", i, equations[i].a, equations[i].m);
    printf("\n");

    // make sure the m's are pairwise co-prime
    for (int i = 0; i < c_numEquations; ++i)
    {
        for (int j = i + 1; j < c_numEquations; ++j)
        {
            int s, t;
            int gcd = ExtendedEuclidianAlgorithm(equations[i].m, equations[j].m, s, t);
            if (gcd != 1)
            {
                printf("%i and %i are not co-prime (index %i and %i)\n", equations[i].m, equations[j].m, i, j);
                WaitForEnter();
                return 0;
            }
        }
    }

    // calculate the coefficients for each term
    std::array < int, c_numEquations > coefficients;
    coefficients.fill(1);
    for (int i = 0; i < c_numEquations; ++i)
    {
        for (int j = 0; j < c_numEquations; ++j)
        {
            if (i != j)
                coefficients[i] *= equations[j].m;
        }
    }

    // now figure out how much to multiply each coefficient by to make it have the specified modulus residue (remainder)
    int result = 0;
    for (int i = 0; i < c_numEquations; ++i)
    {
        int s, t;
        ExtendedEuclidianAlgorithm(coefficients[i], equations[i].m, s, t);
        coefficients[i] *= t * equations[i].a;
    }

    // calculate result and simplify it to the smallest positive integer mod lcm
    // lcm is the product when they are pairwise coprime, as the gcd of any two is 1.
    int lcm = 1;
    for (int i = 0; i < c_numEquations; ++i)
    {
        lcm *= equations[i].m;
        result += coefficients[i];
    }
    result = result % lcm;
    if (result < 0)
        result += lcm;

    // print out the answer
    printf("x = %i mod %i\nor...\n", result, lcm);
    printf("x = %i + %i*N\nWhere N is any positive or negative integer.\n\n", result, lcm);

    // verify that our result is correct
    printf("Verifying Equations:\n");
    for (int i = 0; i < c_numEquations; ++i)
        printf("eq %i:  %i mod %i = %i (%s)\n", i, result, equations[i].m, result % equations[i].m, (result % equations[i].m == equations[i].a) ? "PASS" : "!!FAIL!!");

    WaitForEnter();
    return 0;
}

/*

Blog:
! Solving simultaneous congruencies
! mention it was invented by Sun Tzu, but not the same Sun Tzu who wrote the art of war
! show usage case of CRT with an example solution?
! link to video on chinese remainder theorem: https://www.youtube.com/watch?v=ru7mWZJlRQg
! link to modulus inversion post: http://blog.demofox.org/2015/09/10/modular-multiplicative-inverse/
* do they have to be coprime?
 *  The thing where they don't strictly have to be coprime to be solvable. You could see that you could multiply everything by 5 and the result will be correct but they won't be coprime anymore as an example.
* show some example runs w/ a few different equation sets (two working, one not working?)

Links:
https://en.wikipedia.org/wiki/Chinese_remainder_theorem

*/