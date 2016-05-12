#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <cmath>
#include <array>
#include <assert.h>
#include <inttypes.h>

typedef int64_t TINT;

#define TEST_ACCURACY() 1
#define ACCURACYTEST_TESTCOUNT() 1000
#define ACCURACYTEST_PRIMEMIN()  1000

//=================================================================
struct SPrimeInfo
{
    TINT                m_prime;
    std::array<TINT, 2> m_imaginaries;
};

//=================================================================
struct SIntermediate
{
    SIntermediate (TINT v0 = 0, TINT v1 = 0)
    {
        m_values[0] = v0;
        m_values[1] = v1;
    }

    void Reduce (TINT prime)
    {
        m_values[0] = m_values[0] % prime;
        m_values[1] = m_values[1] % prime;
    }

    std::array<TINT, 2> m_values;
};

//=================================================================
struct SComplex
{
    SComplex (TINT real = 0, TINT imaginary = 0)
    {
        m_real = real;
        m_imaginary = imaginary;
    }

    TINT m_real;
    TINT m_imaginary;
};

//=================================================================
SIntermediate operator + (const SIntermediate &A, const SIntermediate &B)
{
    return SIntermediate(A.m_values[0] + B.m_values[0], A.m_values[1] + B.m_values[1]);
}

//=================================================================
SIntermediate operator * (const SIntermediate &A, const SIntermediate &B)
{
    return SIntermediate(A.m_values[0] * B.m_values[0], A.m_values[1] * B.m_values[1]);
}

//=================================================================
SComplex operator + (const SComplex &A, const SComplex &B)
{
    return SComplex(A.m_real + B.m_real, A.m_imaginary + B.m_imaginary);
}

//=================================================================
SComplex operator * (const SComplex &A, const SComplex &B)
{
    //(a + bi) * (c + di) = ac - bd + (bc + ad)i
    return SComplex(
        A.m_real * B.m_real - A.m_imaginary * B.m_imaginary,
        A.m_imaginary * B.m_real + A.m_real * B.m_imaginary
    );
}

//=================================================================================
bool operator == (const SComplex &A, const SComplex &B)
{
    return A.m_real == B.m_real && A.m_imaginary == B.m_imaginary;
}

//=================================================================================
bool operator != (const SComplex &A, const SComplex &B)
{
    return A.m_real != B.m_real || A.m_imaginary != B.m_imaginary;
}

//=================================================================================
TINT ExtendedEuclidianAlgorithm (TINT smaller, TINT larger, TINT &s, TINT &t)
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
    std::array<TINT, 2> remainders = { larger, smaller };
    std::array<TINT, 2> ss = { 1, 0 };
    std::array<TINT, 2> ts = { 0, 1 };
    size_t indexNeg2 = 0;
    size_t indexNeg1 = 1;
 
    // loop
    while (1)
    {
        // calculate our new quotient and remainder
        TINT newQuotient = remainders[indexNeg2] / remainders[indexNeg1];
        TINT newRemainder = remainders[indexNeg2] - newQuotient * remainders[indexNeg1];
 
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
        TINT newS = ss[indexNeg2] - newQuotient * ss[indexNeg1];
        TINT newT = ts[indexNeg2] - newQuotient * ts[indexNeg1];
 
        // store our values for the next iteration
        remainders[indexNeg2] = newRemainder;
        ss[indexNeg2] = newS;
        ts[indexNeg2] = newT;
 
        // move to the next iteration
        std::swap(indexNeg1, indexNeg2);
    }
}

//=================================================================
TINT MultiplicativeInverse (TINT number, TINT prime)
{
    TINT s, t;
    TINT GCD = ExtendedEuclidianAlgorithm(number, prime, s, t);
    assert(GCD == 1);
    return t;
}

//=================================================================
SIntermediate ComplexToIntermediate (const SComplex& complex, const SPrimeInfo& primeInfo)
{
    SIntermediate ret;
    ret.m_values[0] = complex.m_real + complex.m_imaginary * primeInfo.m_imaginaries[0];
    ret.m_values[1] = complex.m_real + complex.m_imaginary * primeInfo.m_imaginaries[1];
    return ret;
}

//=================================================================
SComplex IntermediateToComplex (const SIntermediate& intermediate, const SPrimeInfo& primeInfo)
{
    // solve for a and b in a + bi

    // start with two equations for our intermediate values:
    // eqn 1   a + primeInfo.m_imaginaries[0] * b = intermediate[0]
    // eqn 2   a + primeInfo.m_imaginaries[1] * b = intermediate[1]

    // eqn 1 becomes:
    // eqn 3   a = intermediate[0] - primeInfo.m_imaginaries[0] * b

    // plug eqn 3 into eqn 2:
    // eqn 4   intermediate[0] - primeInfo.m_imaginaries[0] * b + primeInfo.m_imaginaries[1] * b = intermediate[1]
    // eqn 5   intermediate[0] + b * (primeInfo.m_imaginaries[1] - primeInfo.m_imaginaries[0]) = intermediate[1]
    // eqn 6   b * (primeInfo.m_imaginaries[1] - primeInfo.m_imaginaries[0]) = intermediate[1] - intermediate[0]
    // eqn 7   b = (intermediate[1] - intermediate[0]) * (primeInfo.m_imaginaries[1] - primeInfo.m_imaginaries[0]) ^ 1

    // now that we have a value for b from eqn 7, solve eqn 1 for a and plug the b value in:
    // eqn 8   a = intermediate[0] - primeInfo.m_imaginaries[0] * b

    // solve
    TINT multInverse = MultiplicativeInverse(primeInfo.m_imaginaries[1] - primeInfo.m_imaginaries[0], primeInfo.m_prime);
    TINT b = ((intermediate.m_values[1] - intermediate.m_values[0]) % primeInfo.m_prime) * multInverse;
    b = b % primeInfo.m_prime;
    TINT a = intermediate.m_values[0] - (primeInfo.m_imaginaries[0] * b) % primeInfo.m_prime;
    a = a % primeInfo.m_prime;

    return SComplex(a, b);
}

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
void FindPrimeWithImaginary (TINT minimum, SPrimeInfo& primeInfo)
{
    while (minimum >= 0)
    {
        if (IsPrime(minimum) && HasImaginary(minimum, primeInfo.m_imaginaries))
        {
            primeInfo.m_prime = minimum;
            return;
        }
        ++minimum;
    }
    assert(false);
}

//=================================================================
void TestAccuracy()
{
    // find a prime with values for i, and store / report those values
    SPrimeInfo primeInfo;
    FindPrimeWithImaginary(ACCURACYTEST_PRIMEMIN(), primeInfo);
    printf("prime = %" PRId64 ", imaginaries = %" PRId64 " and %" PRId64 "\n\n", primeInfo.m_prime, primeInfo.m_imaginaries[0], primeInfo.m_imaginaries[1]);

    for (size_t i = 0; i < ACCURACYTEST_TESTCOUNT(); ++i)
    {
        // TODO: randomize inputs to be [0, prime) ? or maybe prime / 2 or something

        // do regular complex arithmetic
        SComplex a(-1, 1);
        SComplex b( 0, 1);
        SComplex c = a * b;

        // do the technique
        SIntermediate A = ComplexToIntermediate(a, primeInfo);
        SIntermediate B = ComplexToIntermediate(b, primeInfo);
        SIntermediate C = A * B;
        C.Reduce(primeInfo.m_prime);
        SComplex result = IntermediateToComplex(C, primeInfo);

        // report mismatches
        if (c != result)
        {
            printf("(%" PRId64 " + %" PRId64 "i) * (%" PRId64 " + %" PRId64 "i) =\n", a.m_real, a.m_imaginary, b.m_real, b.m_imaginary);
            printf("WRONG: %" PRId64 " + %" PRId64 "i\n", result.m_real, result.m_imaginary);
            printf("RIGHT: %" PRId64 " + %" PRId64 "i\n\n", c.m_real, c.m_imaginary);
        }
    }
}

//=================================================================
int main (int argc, char **argv)
{
    #if TEST_ACCURACY()
        TestAccuracy();
    #endif

    // find a prime with values for i, and store the prime and those values
    SPrimeInfo primeInfo;
    FindPrimeWithImaginary(1000, primeInfo);

    // TOOD: temp
    /*
    primeInfo.m_prime = 8837;
    primeInfo.m_imaginaries[0] = 94;
    primeInfo.m_imaginaries[1] = 8743;
    SIntermediate A = ComplexToIntermediate(SComplex(33, 81), primeInfo);
    SIntermediate B = ComplexToIntermediate(SComplex(15, 4), primeInfo);
    */

    // define the complex numbers to multiply and make them into intermediate values
    SIntermediate A = ComplexToIntermediate(SComplex(-1, 1), primeInfo);
    SIntermediate B = ComplexToIntermediate(SComplex(0, 1), primeInfo);

    // reduce the values
    A.Reduce(primeInfo.m_prime);
    B.Reduce(primeInfo.m_prime);

    // do the operation
    SIntermediate C = A * B;

    // reduce the results
    C.Reduce(primeInfo.m_prime);

    // decode the results
    SComplex result = IntermediateToComplex(C, primeInfo);

    // show the prime, imaginaries and result
    printf("prime = %" PRId64 ", imaginaries = %" PRId64 " and %" PRId64 "\n\n", primeInfo.m_prime, primeInfo.m_imaginaries[0], primeInfo.m_imaginaries[1]);
    printf("%" PRId64 " + %" PRId64 "i\n\n", result.m_real, result.m_imaginary);
    WaitForEnter();
    return 0;
}

/*
TODO:
* make it so we can use this to time this method compared to standard methods.
* make a mode (with #define) that does operations with random numbers and compares results with reality

* understand limits of sizes of inputs and outputs due to size of prime
* see if this works with negative values
 * in the decoding i add prime if the results are negative
 * that might be wrong for negative values though.  I think it might be negative if the result is over half of the prime?
 * i stopped doing that, but the modulus inverse now has it. is that wrong?
 * stopped doing that too... need to investigate.  c++ modulus may not be the right behavior mathematically

? is there a way to quickly do complex conjugate while it's in the encoded form? could be a useful operation
*/