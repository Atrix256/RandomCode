#include <stdio.h>
#include <algorithm>
#include <array>

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
            return newQuotient;
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
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
int main (int argc, char **argv)
{
    // get user input
    int a, m;
    printf("Modulus Inverse Calculator:  a*x % m = 1\n");
    printf("a = ?\n");
    scanf("%i", &a);
    printf("m = ?\n");
    scanf("%i", &m);

    // Attempt inverse
    int s, t;
    int GCD = ExtendedEuclidianAlgorithm(a, m, s, t);

    // TODO: 5 and 7 = GCD of 2??! look int it!

    // report failure if we couldn't do inverse
    if (GCD != 1)
    {
        printf("Values are not co-prime, cannot inverse!\n");
        WaitForEnter();
        return 0;
    }

    // Report details of inverse and show that it worked
    printf("%i mod %i = %i\n", a, m, a % m);
    printf("GCD = %i, Inverse = %i\n", GCD, t);
    printf("(%i * %i) mod %i = %i\n", a, t, m, (a*t)%m);
    WaitForEnter();
    return 0;
}

/*

TODO:
* make it work
* blog post
* then chinese remainder theorem, which uses this!

LINKS:
* http://blog.demofox.org/2015/01/24/programmatically-calculating-gcd-and-lcm/
* https://en.wikipedia.org/wiki/Modular_multiplicative_inverse
* https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm

*/