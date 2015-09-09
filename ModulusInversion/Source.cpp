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
void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
int main (int argc, char **argv)
{
    /*
    // Wikipedia example!
    int s, t;
    int ret = ExtendedEuclidianAlgorithm(46, 240, s, t);

    int s2, t2;
    int ret2 = ExtendedEuclidianAlgorithm(240, 46, s2, t2);
    */
    
    // TODO: if EEA return != 1, they are not coprime!
    // TODO: user input to get values to work on?
    // TODO: print out step by step in loop?

    int a = 5;
    int m = 7;

    int s, t;
    int ret = ExtendedEuclidianAlgorithm(5, 7, s, t);

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