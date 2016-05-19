#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <cmath>
#include <array>
#include <assert.h>
#include <inttypes.h>
#include <random>
#include <Windows.h> // for QueryPerformanceCounter

#if 0
typedef int64_t TINT;
#define TINTFORMAT() PRId64
#else
typedef int32_t TINT;
#define TINTFORMAT() "i"
#endif

#define TEST_ACCURACY() 0
#define ACCURACYTEST_TESTCOUNT() 1000
#define ACCURACYTEST_PRIMEMIN()  1000

#define TEST_SPEED() 1
#define SPEEDTEST_TESTCOUNT() 10000000
#define SPEEDTEST_PRIMEMIN() 1000
#define SPEEDTEST_TESTROUNDS() 100   // it's best to do a lot of rounds so the CPU is running at full speed, giving good results
#define SPEEDTEST_ENCODEDECODECOUNT() 50 // does an encode/decode every this many operations

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
    printf("prime = %" TINTFORMAT() ", imaginaries = %" TINTFORMAT() " and %" TINTFORMAT() "\n\n", primeInfo.m_prime, primeInfo.m_imaginaries[0], primeInfo.m_imaginaries[1]);

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
            printf("(%" TINTFORMAT() " + %" TINTFORMAT() "i) * (%" TINTFORMAT() " + %" TINTFORMAT() "i) =\n", a.m_real, a.m_imaginary, b.m_real, b.m_imaginary);
            printf("WRONG: %" TINTFORMAT() " + %" TINTFORMAT() "i\n", result.m_real, result.m_imaginary);
            printf("RIGHT: %" TINTFORMAT() " + %" TINTFORMAT() "i\n\n", c.m_real, c.m_imaginary);
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
    fprintf(file, "\"Standard\",\"Technique\",\"Technique with encode/decode every %i operations\"\n", SPEEDTEST_ENCODEDECODECOUNT());

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

    // NOTE: only profiling multiplies (not adds) because adds are the number of ops in both techniques

    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    // these are to keep it from optimizing away the operations
    SComplex sumComplex;
    SIntermediate sumIntermediate;

    for (size_t roundIndex = 0; roundIndex < SPEEDTEST_TESTROUNDS(); ++roundIndex)
    {
        // do standard complex multiplications
        QueryPerformanceCounter(&start);
        for (size_t i = 0; i < SPEEDTEST_TESTCOUNT(); ++i)
        {
            size_t baseIndex = (i * 2) & c_valueMask;
            SComplex result = c_complexValues[baseIndex] * c_complexValues[baseIndex + 1];
            sumComplex = sumComplex + result;
        }
        QueryPerformanceCounter(&end);
        double standard = ((double)(end.QuadPart - start.QuadPart)) * 1000.0 / freq.QuadPart;

        // do technique based complex multiplications
        QueryPerformanceCounter(&start);
        for (size_t i = 0; i < SPEEDTEST_TESTCOUNT(); ++i)
        {
            size_t baseIndex = (i * 2) & c_valueMask;
            SIntermediate result = c_intermediateValues[baseIndex] * c_intermediateValues[baseIndex + 1];
            sumIntermediate = sumIntermediate + result;
        }
        QueryPerformanceCounter(&end);
        double technique = ((double)(end.QuadPart - start.QuadPart)) * 1000.0 / freq.QuadPart;

        // do technique with encodes and decodes
        static const size_t testCount = SPEEDTEST_TESTCOUNT() / SPEEDTEST_ENCODEDECODECOUNT();
        static const size_t remainderTestCount = SPEEDTEST_TESTCOUNT() % SPEEDTEST_ENCODEDECODECOUNT();
        QueryPerformanceCounter(&start);
        for (size_t j = 0; j < testCount; ++j)
        {
            //do an encode
            sumIntermediate = sumIntermediate + ComplexToIntermediate(c_complexValues[j&c_valueMask], primeInfo);

            //do multiplies
            for (size_t i = 0; i < SPEEDTEST_ENCODEDECODECOUNT(); ++i)
            {
                size_t baseIndex = (i * 2) & c_valueMask;
                SIntermediate result = c_intermediateValues[baseIndex] * c_intermediateValues[baseIndex + 1];
                sumIntermediate = sumIntermediate + result;
            }

            //do a decode
            sumComplex = sumComplex + IntermediateToComplex(sumIntermediate, primeInfo);
        }

        // do remainder of multiplies to reach SPEEDTEST_ENCODEDECODECOUNT()
        for (size_t i = 0; i < remainderTestCount; ++i)
        {
            size_t baseIndex = (i * 2) & c_valueMask;
            SIntermediate result = c_intermediateValues[baseIndex] * c_intermediateValues[baseIndex + 1];
            sumIntermediate = sumIntermediate + result;
        }
        QueryPerformanceCounter(&end);
        double technique2 = ((double)(end.QuadPart - start.QuadPart)) * 1000.0 / freq.QuadPart;

        // print to screen
        printf("\nRound %i\n", roundIndex);
        printf("    Standard : %f ms\r\n", standard);
        printf("    Technique : %f ms\r\n", technique);
        printf("    Technique with encode/decode every %i operations : %f ms\r\n", SPEEDTEST_ENCODEDECODECOUNT(), technique2);

        // write to csv
        fprintf(file, "\"%f\",\"%f\",\"%f\"\n", standard, technique, technique2);
    }

    fclose(file);
    return (size_t)(sumComplex.m_imaginary + sumComplex.m_real + sumIntermediate.m_value0 + sumIntermediate.m_value1);
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
    printf("prime = %" TINTFORMAT() ", imaginaries = %" TINTFORMAT() " and %" TINTFORMAT() "\n\n", primeInfo.m_prime, primeInfo.m_imaginaries[0], primeInfo.m_imaginaries[1]);
    printf("%" TINTFORMAT() " + %" TINTFORMAT() "i\n\n", result.m_real, result.m_imaginary);
    */
    
    WaitForEnter();
    return 0;
}

/*
TODO:
* make the accuracy test work (+ / - has ambiguity right now)
* make the accuracy test use random numbers for it's work

* compare perf in x64?

* understand limits of sizes of inputs and outputs due to size of prime
* see if this works with negative values
 * in the decoding i add prime if the results are negative
 * that might be wrong for negative values though.  I think it might be negative if the result is over half of the prime?
 * i stopped doing that, but the modulus inverse now has it. is that wrong?
 * stopped doing that too... need to investigate.  c++ modulus may not be the right behavior mathematically

? compare the timing with int32 instead of int64.

? is there a way to quickly do complex conjugate while it's in the encoded form? could be a useful operation

? can we extend to higher order imaginary numbers? like i^4 = -1?

? can we do it with 1 intermediate value, instead of two paralelized values?

? what are other hypercomplex multiplication methods?

? can we speed this up further with SIMD operations?  I'll bet we could!

*/

/*

? how did matt intend on using karatsuba for our stuff?  can you figure out from an operations point of view whether it's faster or slower? fewer multiplies or anything?
 * M1 = a_r * b_r
 * M2 = a_i * b_i
 * M3 = (a_r * a_i) + (b_r * b_i)
 * Real = M1+M2
 * Imaginary = **Something** - (M1 + M2) (?)
 ! need to think through this some more


* understand this https://en.wikipedia.org/wiki/Cayley%E2%80%93Dickson_construction, and how it might apply

? are int32's faster than int64? test it! win32 and x64
 * in win32 yes, int32's are way faster, but the speed comparisons are very similar.
 * win32 64 bit int, 10 million ops: ~100ms for technique.  ~160ms for standard or 50 ops between encode / decode.  ~63% time.
 * win32 32 bit int, 10 million ops: ~8ms for technique.  ~24ms for standard or 50 ops between encode / decode   ~33% time.

? what is x64 values like above.  list here, so it is easy to digest.

? compare number of operations between standard / technique.  Does 50 ops seem like the same amount of work? memory access, multiply add operations, and other factors make it not quite apples to apples but check it out.
 * standard multiply = 4 multiply, 2 adds.
 * technique multiply = 2 multiply.

? simd help?
? maybe we just say "a complex multiplication is two multiplies, instead of 4 multiplies and 3 adds".  That is proof enough that it is an improvement imo
? x64 seems to show no better in this technique vs standard.  can we explain why not?

Email to Matt:

It turns out that the optimizer was eating up a lot of the work i was profiling, despite efforts to fight that.  With that fixed, there's still a clear win of your technique vs the standard way, as we would expect, it just turns out that both the standard way and your technique take a lot longer than my last reported values.

I also found what seems to be the break even point for how many operations you have to do between a decode and an encode and it looks like the answer is 50.  So, if you do an encode, then 50 of your multiplies, then a decode, that is about the same speed as doing 50 standard complex number multiplies.  Anything above 50 multiplies is faster.  Kind of a lot of operations for the break even unfortunately!!  It is still a win though, so does seem to be like good results people would be interested in.  If we can get higher order imaginary numbers working, i'll bet the break even point is lower :P

Each data sample is still 10 million multiplies.

[image of graph for w32 values]

.. talk about x64 and the rest


*/

/*
!! it looks like i^8 values do in fact sum up to 0 mod p.  Infact, it looks like they sum up to 4*p always?!
 ! i^4 seems to sum up to 2p
 ! i^2 sums up to p
 ! these things actually make sense, because there are n numbers of answers, when there are answers. the answers come in pairs, where each is the negative of the other, so they sum to p.
   * so, for i^n mod p, they should sum to (n/2)*p
? how can we use this to more quickly find i's for prime numbers?



----- ? do the techniques work with higher order imaginary numbers? -----

p = 17
i0 = 2^4 = -1 (16)
i1 = 8^4 = -1 (4096)
i2 = 9^4 = -1 (6561)
i3 = 15^4 = -1 (50625)

p = 41
i0 = 3^4 = -1 (81)
i1 = 14^4 = -1 (38416)
i2 = 27^4 = -1 (531441)
i3 = 38^4 = -1 (2085136)

p = 73
i0 = 10^4 = -1 (10000)
i1 = 22^4 = -1 (234256)
i2 = 51^4 = -1 (6765201)
i3 = 63^4 = -1 (15752961)

p = 89
i0 = 12^4 = -1 (20736)
i1 = 37^4 = -1 (1874161)
i2 = 52^4 = -1 (7311616)
i3 = 77^4 = -1 (35153041)

i0 + i3 = -1
i1 + i2 = -1

? what is relation between i0 and i1 if any? same question for i2 and i3?

* Wrong relations:
 * i0*i1 = -1
 * i2*i3 = -1
 * this works for p=17 and p=89, but not 41 or 73, for those it's +1!  (a little weird)

----- ? how do we decode an i^4 number? ------

* with i^2, we have two parallelized equations where you add them to get 2a and subtract to get 2bi

* i3 = -i0
* i2 = -i1
? is there a relation between i0 and i1, or between i2 and i3? if so might help us solve the equations better

We have 4 parallelized equations:
1) a + bi_0 + ci_0^2 + di_0^3
2) a + bi_1 + ci_1^2 + di_1^3
3) a + bi_2 + ci_2^2 + di_2^3
4) a + bi_3 + ci_3^2 + di_3^3

also known as:
1) a + bi_0 + ci_0^2 + di_0^3
2) a + bi_1 + ci_1^2 + di_1^3
3) a + b(-i_1) + c(-i_1)^2 + d(-i_1)^3
4) a + b(-i_0) + c(-i_0)^2 + d(-i_0)^3

! now we must solve for a,b,c,d!

adding eq1 and eq4 we get: (useful)
(a + bi_0 + ci_0^2 + di_0^3) + (a + b(-i_0) + c(-i_0)^2 + d(-i_0)^3) = 4a

subtracting eq4 from eq1 we get: (not sure if useful)
(a + bi_0 + ci_0^2 + di_0^3) - (a + b(-i_0) + c(-i_0)^2 + d(-i_0)^3) = 2bi_0 + 2ci_0^2, + 2di_0^3

*/