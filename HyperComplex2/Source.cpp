#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <cmath>
#include <array>
#include <assert.h>

typedef int64_t TINT;

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

    // a + bi
    TINT b = (intermediate.m_values[1] - intermediate.m_values[0]) * MultiplicativeInverse(primeInfo.m_imaginaries[1] - primeInfo.m_imaginaries[0], primeInfo.m_prime);
    b = b % primeInfo.m_prime;
    TINT a = intermediate.m_values[0] - primeInfo.m_imaginaries[0] * b;

    // TODO: this
    SComplex ret(a, b);
    
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
int main (int argc, char **argv)
{
    // find a prime with values for i, and store the prime and those values
    SPrimeInfo primeInfo;
    FindPrimeWithImaginary(100, primeInfo);

    // TOOD: temp
    primeInfo.m_prime = 8837;
    primeInfo.m_imaginaries[0] = 94;
    primeInfo.m_imaginaries[1] = 8743;

    // define the complex numbers to multiply and make them into intermediate values
    //SIntermediate A = ComplexToIntermediate(SComplex(1, 1), primeInfo);
    //SIntermediate B = ComplexToIntermediate(SComplex(0, 1), primeInfo);
 
    // TODO: temp
    SIntermediate A = ComplexToIntermediate(SComplex(33, 81), primeInfo);
    SIntermediate B = ComplexToIntermediate(SComplex(15, 4), primeInfo);

    // reduce the values
    A.Reduce(primeInfo.m_prime);
    B.Reduce(primeInfo.m_prime);

    // do the operation
    SIntermediate C = A * B;

    // reduce the results
    C.Reduce(primeInfo.m_prime);

    // decode the results
    SComplex results = IntermediateToComplex(C, primeInfo);

    // show the results
    printf("%i + %ii\n\n", results.m_real, results.m_imaginary);
    WaitForEnter();
    return 0;
}

/*
TODO:
* Get this working with the example data
* test other computations
* make it so we can use this to time this method compared to standard methods.

? is there a way to quickly do complex conjugate?
*/