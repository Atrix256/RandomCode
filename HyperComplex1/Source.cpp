#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <cmath>

typedef int64_t TINT;

//=================================================================
//                NUMBER FILTERING FUNCTIONS
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
bool IsNotPrime (TINT n)
{
    return !IsPrime(n);
}

//=================================================================
bool IsOdd (TINT n)
{
    return (n & 1) != 0;
}

//=================================================================
bool IsAnyNumber (TINT n)
{
    return true;
}

//=================================================================
//                NUMBER TESTING FUNCTIONS
//=================================================================
template <size_t DEGREE>
bool HasImaginary (TINT n)
{
    bool ret = false;
    for (TINT i = 0; i < n; ++i)
    {
        TINT testRaw = 1;
        for (size_t deg = 0; deg < DEGREE; ++deg)
            testRaw *= i;
        TINT test = testRaw % n;
        
        if (test + 1 == n)
        {
            if (!ret)
                printf("p = %"PRId64"\n", n);
            printf("  %"PRId64"^%i = -1 (%"PRId64")\n", i, DEGREE, testRaw);
            ret = true;
        }
    }
    return ret;
}

//=================================================================
template <size_t DEGREE>
bool HasDual (TINT n)
{
    bool ret = false;
    for (TINT i = 0; i < n; ++i)
    {
        // dual numbers are not zero but square (etc) to zero
        if (i % n == 0)
            continue;

        TINT testRaw = 1;
        for (size_t deg = 0; deg < DEGREE; ++deg)
            testRaw *= i;
        TINT test = testRaw % n;
        
        if (test == 0)
        {
            if (!ret)
                printf("p = %"PRId64"\n", n);
            printf("  %"PRId64"^%i = 0 (%"PRId64")\n", i, DEGREE, testRaw);
            ret = true;
        }
    }
    return ret;
}

//=================================================================
template <size_t DEGREE>
bool HasHyperbolic (TINT n)
{
    bool ret = false;
    for (TINT i = 0; i < n; ++i)
    {
        // hyperbolic numbers are not one but square (etc) to one
        if (i % n == 1)
            continue;

        TINT testRaw = 1;
        for (size_t deg = 0; deg < DEGREE; ++deg)
            testRaw *= i;
        TINT test = testRaw % n;
        
        if (test == 1)
        {
            if (!ret)
                printf("p = %"PRId64"\n", n);
            printf("  %"PRId64"^%i = 1 (%"PRId64")\n", i, DEGREE, testRaw);
            ret = true;
        }
    }
    return ret;
}

//=================================================================
//                        PROGRAM
//=================================================================
void WaitForEnter ()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================
template <TINT NUMSTART, TINT NUMEND, typename FILTER, typename TEST>
void ProcessNumbers (const FILTER& filter, const TEST& test)
{
    TINT numTotal = 0;
    TINT numPassedFilter = 0;
    TINT numPassedTest = 0;

    for (TINT i = NUMSTART; i <= NUMEND; ++i)
    {
        ++numTotal;

        if (!filter(i))
            continue;

        ++numPassedFilter;

        if (!test(i))
            continue;

        ++numPassedTest;
    }

    printf("\n%"PRId64" numbers passed the filter out of %"PRId64" tested (%0.0f%%)\n", numPassedFilter, numTotal, 100.0f * float(numPassedFilter) / float(numTotal));
    printf("%"PRId64" numbers passed the test out of those that passed the filter (%0.0f%%)\n\n", numPassedTest, 100.0f * float(numPassedTest) / float(numPassedFilter));
}

//=================================================================
int main (int argc, char **argv)
{
    ProcessNumbers<0, 1000>(IsPrime, HasImaginary<4>);

    WaitForEnter();
    return 0;
}

/*
TODO:
* understand paper more, share results, think about next stuff.

Questions:
* how can you have a dual number? something where the number is not 0, but you square it to get 0?
* where does fft fit in?
* how do you do multiple calculations simultaneously?  It seems like it'd be similar to what I do.
* are we really restricted to primes? or can we limit to odd numbers or something else less restrictive? (more solutions = smaller numbers used)

-----------------------------------------------------------------------------
                                   NOTES
-----------------------------------------------------------------------------

* we may want to do multi precision ints if we want to try higher powers or larger numbers.  boost multiprecision library is easy to use, so is there if we need or want it.

----- IMAGINARY NUMBERS -----
* Yeah... your thing about probability of numbers tested seems to fit!
 * when looking at prime numbers from 0 to 100: 
  * HasImaginary<2>: 48% had i^2 = -1  (12/25)   code: ProcessNumbers<0, 100>(IsPrime, HasImaginary<2>);
  * HasImaginary<4>: 24% had i^4 = -1  ( 6/25)   code: ProcessNumbers<0, 100>(IsPrime, HasImaginary<4>);
  * HasImaginary<8>: 12% had i^8 = -1  ( 3/25)   code: ProcessNumbers<0, 100>(IsPrime, HasImaginary<8>);
  * HasImaginary<16>:  didn't follow the pattern but im pretty sure int64 is not large enough for this test as i've written it so is a numerical error problem.
* There is something weird with non power of two imaginary testing
 * Odd numbered powers find something to do i^n = -1, 100% of the time.  Totally expected.
 * HasImaginary<6> has a 48% (12/25) find rate. 12 primes used as p, have i^6=-1 values.
   * 5 of them had 6 solutions
   * 6 of them had 2 solutions
   * 1 of them had 1 solution (p = 2)
   * code: ProcessNumbers<0, 100>(IsPrime, HasImaginary<6>);
* something weird... the solutions for HasImaginary<2> seem to add up to p!

----- DUAL NUMBERS -----
* No prime numbers from 0 to 10,000 seem to have any solutions for dual numbers
 * ProcessNumbers<0, 10000>(IsPrime, HasDual<2>)
* Non primes do though of course!
 * ProcessNumbers<0, 100>(IsAnyNumber, HasDual<2>);

----- HYPERBOLIC NUMBERS -----
* Much higher success rate than imaginary numbers
 * ProcessNumbers<0, 100>(IsPrime, HasHyperbolic<2>) = 96% of the primes had a solution
 * ProcessNumbers<0, 100>(IsPrime, HasHyperbolic<4>) = 96% again
 * ProcessNumbers<0, 100>(IsPrime, HasHyperbolic<6>) = 96%
 * ProcessNumbers<0, 100>(IsPrime, HasHyperbolic<8>) = 96%
 * ProcessNumbers<0, 10000>(IsPrime, HasHyperbolic<8>) = 49% 
* When there is an answer, it is prime-1.
 * there are also sometimes more answers, but they add up in pairs to prime.

-----------------------------------------------------------------------------
                                 SOLUTIONS
-----------------------------------------------------------------------------

Here are some example solutions

----- IMAGINARY NUMBERS (DEGREE 2) PRIMES ONLY -----

p = 5
 2^2 = 4
 3^2 = 9
 
p = 13
 5^2 = 25
 8^2 = 64

....

p = 89
 34^2 = 1156
 55^2 = 3025

p = 97
 22^2 = 484
 75^2 = 5625

----- IMAGINARY NUMBERS (DEGREE 4) PRIMES ONLY -----

p = 17
 2^4 = 16
 8^4 = 4096
 9^4 = 6561
 15^4 = 50625

....

p = 97
 33^4 = 1185921
 47^4 = 4879681
 50^4 = 6250000
 64^4 = 16777216

----- IMAGINARY NUMBERS (DEGREE 8) PRIMES ONLY -----

p = 17
  3^8 = 6561
  5^8 = 390625
  6^8 = 1679616
  7^8 = 5764801
  10^8 = 100000000
  11^8 = 214358881
  12^8 = 429981696
  14^8 = 1475789056

p = 97
  8^8 = 16777216
  12^8 = 429981696
  18^8 = 11019960576
  27^8 = 282429536481
  70^8 = 576480100000000
  79^8 = 1517108809906561
  85^8 = 2724905250390625
  89^8 = 3936588805702081

----- HYPERBOLIC NUMBERS (DEGREE 2) PRIMES ONLY -----

p = 3
  2^2 = 1 (4)
p = 5
  4^2 = 1 (16)
p = 7
  6^2 = 1 (36)
p = 11
  10^2 = 1 (100)

....

p = 79
  78^2 = 1 (6084)
p = 83
  82^2 = 1 (6724)
p = 89
  88^2 = 1 (7744)
p = 97
  96^2 = 1 (9216)

----- HYPERBOLIC NUMBERS (DEGREE 4) PRIMES ONLY -----

p = 3
  2^4 = 1 (16)
p = 5
  2^4 = 1 (16)
  3^4 = 1 (81)
  4^4 = 1 (256)
p = 7
  6^4 = 1 (1296)
p = 11
  10^4 = 1 (10000)

....

p = 79
  78^4 = 1 (37015056)
p = 83
  82^4 = 1 (45212176)
p = 89
  34^4 = 1 (1336336)
  55^4 = 1 (9150625)
  88^4 = 1 (59969536)
p = 97
  22^4 = 1 (234256)
  75^4 = 1 (31640625)
  96^4 = 1 (84934656)

----- HYPERBOLIC NUMBERS (DEGREE 8) PRIMES ONLY -----

p = 3
  2^8 = 1 (256)
p = 5
  2^8 = 1 (256)
  3^8 = 1 (6561)
  4^8 = 1 (65536)
p = 7
  6^8 = 1 (1679616)
p = 11
  10^8 = 1 (100000000)

....

p = 79
  78^8 = 1 (1370114370683136)
p = 83
  82^8 = 1 (2044140858654976)
p = 89
  12^8 = 1 (429981696)
  34^8 = 1 (1785793904896)
  37^8 = 1 (3512479453921)
  52^8 = 1 (53459728531456)
  55^8 = 1 (83733937890625)
  77^8 = 1 (1235736291547681)
  88^8 = 1 (3596345248055296)
p = 97
  22^8 = 1 (54875873536)
  33^8 = 1 (1406408618241)
  47^8 = 1 (23811286661761)
  50^8 = 1 (39062500000000)
  64^8 = 1 (281474976710656)
  75^8 = 1 (1001129150390625)
  96^8 = 1 (7213895789838336)

*/