#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <cmath>
#include <array>
#include <assert.h>
#include <inttypes.h>
#include <random>
#include <Windows.h> // for QueryPerformanceCounter

typedef int64_t TINT;

#define TEST_ACCURACY() 0
#define ACCURACYTEST_TESTCOUNT() 1000
#define ACCURACYTEST_PRIMEMIN()  1000

#define TEST_SPEED() 1
#define SPEEDTEST_TESTCOUNT() 10000000
#define SPEEDTEST_PRIMEMIN() 1000
#define SPEEDTEST_TESTROUNDS() 100   // it's best to do a lot of rounds so the CPU is running at full speed, giving good results

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
        m_value0 = v0;
        m_value1 = v1;
    }

    void Reduce (TINT prime)
    {
        m_value0 = m_value0 % prime;
        m_value1 = m_value1 % prime;
    }

    TINT m_value0;
    TINT m_value1;

    // NOTE: the perf tests took 4x the time when using a std::array instead of members!
    //std::array<TINT, 2> m_values;
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
inline SIntermediate operator + (const SIntermediate &A, const SIntermediate &B)
{
    return SIntermediate(A.m_value0 + B.m_value0, A.m_value1 + B.m_value1);
}

//=================================================================
inline SIntermediate operator * (const SIntermediate &A, const SIntermediate &B)
{
    return SIntermediate(A.m_value0 * B.m_value0, A.m_value1 * B.m_value1);
}

//=================================================================
inline SComplex operator + (const SComplex &A, const SComplex &B)
{
    return SComplex(A.m_real + B.m_real, A.m_imaginary + B.m_imaginary);
}

//=================================================================
inline SComplex operator * (const SComplex &A, const SComplex &B)
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
    ret.m_value0 = complex.m_real + complex.m_imaginary * primeInfo.m_imaginaries[0];
    ret.m_value1 = complex.m_real + complex.m_imaginary * primeInfo.m_imaginaries[1];
    return ret;
}

//=================================================================
SComplex IntermediateToComplex (const SIntermediate& intermediate, const SPrimeInfo& primeInfo)
{
    // solve for a and b in a + bi.  AKA get the real and imaginary components back out
    // 
    // our intermediate values represent:
    // 1) a+bi
    // 2) a+b(-i)
    // where i and -i are the imaginary numbers, which are in fact always negative versions of each other.
    //
    // To get real component, we add the intermediary values and then multiply by 2^(-1) mod p
    //   a + bi
    // + a + b(-i)
    // ___________
    //  2a
    //
    // To get imaginary component, we subtract the intermediary values and then multiply by 2^(-1) mod p
    //   a + bi
    // - a + b(-i)
    // ___________
    //      2bi


    TINT multInverse2 = MultiplicativeInverse(2, primeInfo.m_prime);
    TINT multInverse2i = MultiplicativeInverse(2 * primeInfo.m_imaginaries[0], primeInfo.m_prime);

    if (multInverse2 < 0)
        multInverse2 += primeInfo.m_prime;

    if (multInverse2i < 0)
        multInverse2i += primeInfo.m_prime;

    TINT real = (intermediate.m_value0 + intermediate.m_value1) % primeInfo.m_prime;
    real = (real * multInverse2) % primeInfo.m_prime;

    TINT imaginary = (intermediate.m_value0 - intermediate.m_value1) % primeInfo.m_prime;
    imaginary = (imaginary * multInverse2i) % primeInfo.m_prime;

    return SComplex(real, imaginary);

    /*
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
    */
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
size_t TestSpeed ()
{
    // make csv file and csv file headers
    FILE *file = fopen("speed.csv", "w+t");
    if (!file)
        return 0;
    fprintf(file, "\"Standard\",\"Technique\"\n");

    // find a prime with values for i, and store those values
    SPrimeInfo primeInfo;
    FindPrimeWithImaginary(SPEEDTEST_PRIMEMIN(), primeInfo);

    // calculate random complex numbers for the operation
    std::array<int, std::mt19937::state_size> seed_data;
    std::random_device r;
    std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    std::mt19937 gen(seq);
    std::uniform_int_distribution<TINT> dis(1, SPEEDTEST_PRIMEMIN() - 1);
    SComplex c_complexValues[1024 * 2];
    SIntermediate c_intermediateValues[1024 * 2];
    const size_t c_valueMask = 2048 - 1;
    for (size_t i = 0; i < 2048; ++i)
    {
        c_complexValues[i].m_imaginary = dis(gen);
        c_complexValues[i].m_real = dis(gen);

        c_intermediateValues[i] = ComplexToIntermediate(c_complexValues[i], primeInfo);
    }

    // NOTE: only profiling multiplies because adds are the number of ops in both techniques

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    size_t ret = 0; // this is to keep it from optimizing away the operations
    for (size_t roundIndex = 0; roundIndex < SPEEDTEST_TESTROUNDS(); ++roundIndex)
    {
        printf("\nRound %i\n", roundIndex);
        
        QueryPerformanceCounter(&start);
        for (size_t i = 0; i < SPEEDTEST_TESTCOUNT(); ++i)
        {
            size_t baseIndex = (i * 2) & c_valueMask;
            SComplex result = c_complexValues[baseIndex] * c_complexValues[baseIndex + 1];
            ret += (size_t)result.m_imaginary;
        }
        QueryPerformanceCounter(&end);
        double standard = ((double)(end.QuadPart - start.QuadPart)) * 1000.0 / freq.QuadPart;
        printf("    Standard : %f ms\r\n", standard);
        

        QueryPerformanceCounter(&start);
        for (size_t i = 0; i < SPEEDTEST_TESTCOUNT(); ++i)
        {
            size_t baseIndex = (i * 2) & c_valueMask;
            SIntermediate result = c_intermediateValues[baseIndex] * c_intermediateValues[baseIndex + 1];
            ret += (size_t)result.m_value0;
        }
        QueryPerformanceCounter(&end);
        double technique = ((double)(end.QuadPart - start.QuadPart)) * 1000.0 / freq.QuadPart;
        printf("    Technique : %f ms\r\n", technique);

        // write to csv
        fprintf(file, "\"%f\",\"%f\"\n", standard, technique);
    }

    fclose(file);
    return ret;
}

//=================================================================
int main (int argc, char **argv)
{
    #if TEST_ACCURACY()
        TestAccuracy();
    #endif

    #if TEST_SPEED()
        TestSpeed();
    #endif

        /*
    // find a prime with values for i, and store the prime and those values
    SPrimeInfo primeInfo;
    FindPrimeWithImaginary(1000, primeInfo);

    // TOOD: temp
    primeInfo.m_prime = 8837;
    primeInfo.m_imaginaries[0] = 94;
    primeInfo.m_imaginaries[1] = 8743;
    SIntermediate A = ComplexToIntermediate(SComplex(33, 81), primeInfo);
    SIntermediate B = ComplexToIntermediate(SComplex(15, 4), primeInfo);
    // (33+81i) * (15+4i) = 171 + 1347i

    // define the complex numbers to multiply and make them into intermediate values
    //SIntermediate A = ComplexToIntermediate(SComplex(-1, 1), primeInfo);
    //SIntermediate B = ComplexToIntermediate(SComplex(0, 1), primeInfo);

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
    */
    
    WaitForEnter();
    return 0;
}

/*
TODO:
* make it so you can include encoding and decoding into perf results.  Maybe specify how often you want to do an encode and decode.
* make the accuracy test work (+ / - has ambiguity right now)
* make the accuracy test use random numbers for it's work
* make the perf test create a csv file with results?

* understand limits of sizes of inputs and outputs due to size of prime
* see if this works with negative values
 * in the decoding i add prime if the results are negative
 * that might be wrong for negative values though.  I think it might be negative if the result is over half of the prime?
 * i stopped doing that, but the modulus inverse now has it. is that wrong?
 * stopped doing that too... need to investigate.  c++ modulus may not be the right behavior mathematically

? is there a way to quickly do complex conjugate while it's in the encoded form? could be a useful operation
*/