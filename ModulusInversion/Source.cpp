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
    // get user input
    int a, m, n;
    printf("Given a, m and n, solves for X.\n(a * X) %% m = n\n\n");
    printf("a = ");
    scanf("%i", &a);
    printf("m = ");
    scanf("%i", &m);
    printf("n = ");
    scanf("%i", &n);

    // show details of what they entered
    printf("\n(%i * X) mod %i = %i\n", a, m, n);

    // Attempt brute force
    printf("\nBrute Force Testing X from 0 to %i:\n", (m-1));
    for (int i = 0; i < m; ++i) {
        if ((a*i) % m == n)
        {
            printf("  X = %i\n", i);
            printf("  %i mod %i = %i\n", a*i, m, (a*i) % m);
            break;
        }
        else if (i == (m - 1))
        {
            printf("  No solution!\n");
        }
    }

    // Attempt inverse via Extended Euclidean Algorithm
    printf("\nExtended Euclidean Algorithm:\n");
    int s, t;
    int GCD = ExtendedEuclidianAlgorithm(a, m, s, t);

    // report failure if we couldn't do inverse
    if (GCD != 1)
    {
        printf("  Values are not co-prime, cannot invert! GCD = %i\n", GCD);
    }
    // Else report details of inverse and show that it worked
    else
    {
        printf("  Inverse = %i\n", t);
        printf("  X = Inverse * n = %i\n", t*n);
        printf("  %i mod %i = %i\n", a*t*n, m, (a*t*n) % m);
    }

    WaitForEnter();
    return 0;
}

/*

TODO:
* blog post
* then chinese remainder theorem, which uses this!

Blog:
* Note that this post is a prerequisite for a future post
* talk about brute force and extended euclidian algorithm both5
* show a simple working run
 * 7,9,2
* show a large number run
 * 7, 1000001, 538
* show a run where inverse and then multiply is not the smallest value
 * 5,7,3
* show something that works via brute force but isn't invertible.  Like they aren't coprime but you want the mod to equal two anyways so doesnt matter.
 * a,m,n = 8, 6, 4

LINKS:
* http://blog.demofox.org/2015/01/24/programmatically-calculating-gcd-and-lcm/
* https://en.wikipedia.org/wiki/Modular_multiplicative_inverse
* https://en.wikipedia.org/wiki/Extended_Euclidean_algorithm

*/