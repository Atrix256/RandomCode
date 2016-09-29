#include <stdio.h>
#include <array>
#include <vector>
#include <random>
#include <stdint.h>
#include <limits>
#include <chrono>
#include <algorithm>

// run time parameters
#define GENERATE_CODE()   0
#define DO_TEST()         1
#define VERBOSE_SAMPLES() 0  // Turn on to show info for each sample
static const size_t c_testRepeatCount = 10000;
static const size_t c_testSamples = 50;

// code generation parameters
typedef uint32_t TTestType;
#define TTestTypeName "uint32_t"
#define TTestTypePrintf "%u"
static const size_t c_generateNumValues = 5000;
static const size_t c_sparseNumValues = 1000;
static_assert(c_sparseNumValues <= c_generateNumValues,"c_sparseNumValues must be less than or equation to c_generateNumValues.");

// collects multiple timings of a block of code and calculates the average and standard deviation (variance)
// incremental average and variance algorithm taken from: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
struct SBlockTimerAggregator {
    SBlockTimerAggregator(const char* label)
        : m_label(label)
        , m_numSamples(0)
        , m_mean(0.0f)
        , m_M2(0.0f)
    {

    }

    void AddSample (float milliseconds)
    {
        ++m_numSamples;
        float delta = milliseconds - m_mean;
        m_mean += delta / float(m_numSamples);
        m_M2 += delta * (milliseconds - m_mean);
    }

    ~SBlockTimerAggregator ()
    {
        printf("%s: avg = %0.2f ms. std dev = %0.2f ms\n", m_label, GetAverage(), GetStandardDeviation());
    }

    float GetAverage () const
    {
        return m_mean;
    }

    float GetVariance () const
    {
        // invalid!
        if (m_numSamples < 2)
            return -1.0f;
        
        return m_M2 / float (m_numSamples - 1);
    }

    float GetStandardDeviation () const
    {
        return sqrt(GetVariance());
    }

    const char* m_label;

    int         m_numSamples;
    float       m_mean;
    float       m_M2;
};

// times a block of code
struct SBlockTimer
{
    SBlockTimer(SBlockTimerAggregator& aggregator)
        : m_aggregator(aggregator)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~SBlockTimer()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        float milliseconds = seconds.count() * 1000.0f;
        m_aggregator.AddSample(milliseconds);

        #if VERBOSE_SAMPLES()
        printf("%s: %0.2f ms  (avg = %0.2f ms. std dev = %0.2f ms) \n", m_aggregator.m_label, milliseconds, m_aggregator.GetAverage(), m_aggregator.GetStandardDeviation());
        #endif
    }

    SBlockTimerAggregator&                m_aggregator;
    std::chrono::system_clock::time_point m_start;
};

TTestType Random()
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static const TTestType min = std::numeric_limits<TTestType>::min();
    static const TTestType max = std::numeric_limits<TTestType>::max();
    static std::uniform_int_distribution<TTestType> dist(min, max);
    return dist(mt);
}

void GenerateCode ()
{

    FILE *file = fopen("Tests.h", "w+t");
    if (!file)
        return;

    // generate the numbers
    std::vector<TTestType> randomValues;
    randomValues.resize(c_generateNumValues);
    for (TTestType& v : randomValues)
        v = Random();

    // print out how many values there are
    fprintf(file, "static const size_t c_numValues = %u;\n\n", c_generateNumValues);

    // make a shuffle order array
    std::random_device rd;
    std::mt19937 g(rd());
    std::vector<size_t> shuffleOrder;
    shuffleOrder.resize(c_generateNumValues);
    for (size_t i = 0; i < c_generateNumValues; ++i)
        shuffleOrder[i] = i;
    std::shuffle(shuffleOrder.begin(), shuffleOrder.end(), g);
    fprintf(file, "static const size_t c_shuffleOrder[%u] = {", c_generateNumValues);
    fprintf(file, TTestTypePrintf, shuffleOrder[0]);
    for (size_t index = 1; index < c_generateNumValues; ++index)
        fprintf(file, ","TTestTypePrintf, shuffleOrder[index]);
    fprintf(file, "};\n\n");

    // make a sparse shuffle order array
    fprintf(file, "static const size_t c_sparseShuffleOrder[%u] = {", c_sparseNumValues);
    fprintf(file, TTestTypePrintf, shuffleOrder[0]);
    for (size_t index = 1; index < c_sparseNumValues; ++index)
        fprintf(file, ","TTestTypePrintf, shuffleOrder[index]);
    fprintf(file, "};\n\n");

    // make a c style array
    fprintf(file, "static const " TTestTypeName " carray[%u] = {", c_generateNumValues);
    fprintf(file, TTestTypePrintf, randomValues[0]);
    for (size_t index = 1; index < c_generateNumValues; ++index)
        fprintf(file, ","TTestTypePrintf,randomValues[index]);
    fprintf(file,"};\n\n");

    // make a std::array style array
    fprintf(file, "static const std::array<" TTestTypeName ", %u> stdarray = {", c_generateNumValues);
    fprintf(file, TTestTypePrintf, randomValues[0]);
    for (size_t index = 1; index < c_generateNumValues; ++index)
        fprintf(file, ","TTestTypePrintf, randomValues[index]);
    fprintf(file, "}; \n\n");

    // make a std::vector style array
    fprintf(file, "static const std::vector<" TTestTypeName "> stdvector = {");
    fprintf(file, TTestTypePrintf, randomValues[0]);
    for (size_t index = 1; index < c_generateNumValues; ++index)
        fprintf(file, ","TTestTypePrintf, randomValues[index]);
    fprintf(file, "}; \n\n");

    // make the switch statement function
    fprintf(file, "inline " TTestTypeName " LookupSwitch (size_t index) {\n");
    fprintf(file, "    switch(index) {\n");
    for (size_t index = 0; index < c_generateNumValues; ++index)
        fprintf(file, "        case %u: return " TTestTypePrintf ";\n", index, randomValues[index]);
    fprintf(file, "    }\n    ((int*)0)[0] = 0; return -1;\n}\n\n");

    // make the sparse switch statement function
    std::vector<size_t> sparseRandomValues;
    for (size_t i = 0; i < c_generateNumValues; ++i)
    {
        if (std::find(&shuffleOrder[0], &shuffleOrder[c_sparseNumValues], i) != &shuffleOrder[c_sparseNumValues])
            sparseRandomValues.push_back(i);
    }
    fprintf(file, "inline " TTestTypeName " SparseLookupSwitch (size_t index) {\n");
    fprintf(file, "    switch(index) {\n");
    for (size_t index = 0; index < c_sparseNumValues; ++index)
        fprintf(file, "        case %u: return " TTestTypePrintf ";\n", sparseRandomValues[index], randomValues[sparseRandomValues[index]]);
    fprintf(file, "    }\n    ((int*)0)[0] = 0; return -1;\n}\n");

    // close the file
    fclose(file);
}

#if DO_TEST()
    #include "Tests.h"
#endif

int main(int argc, char **argv)
{
    #if GENERATE_CODE()
        // To make the comparison code
        GenerateCode();
    #endif

    #if DO_TEST()
        // make our dynamic memory
        TTestType* dynamicmemory = new TTestType[c_numValues];
        memcpy(dynamicmemory, &stdvector[0], sizeof(TTestType)*c_numValues);

        // Sequential Sum Tests
        TTestType sum = 0;
        {
            printf("Sequential Sum Timings:\n");

            {
                SBlockTimerAggregator timerAgg("  std::vector   ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += stdvector[i];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  std::array    ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += stdarray[i];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  c array       ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += carray[i];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  dynamic memory");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += dynamicmemory[i];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  Switch        ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += LookupSwitch(i);
                }
            }
        }

        // shuffle sum tests
        {
            printf("\nShuffle Sum Timings:\n");

            {
                SBlockTimerAggregator timerAgg("  std::vector   ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += stdvector[c_shuffleOrder[i]];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  std::array    ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += stdarray[c_shuffleOrder[i]];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  c array       ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += carray[c_shuffleOrder[i]];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  dynamic memory");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += dynamicmemory[c_shuffleOrder[i]];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  Switch        ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_numValues; ++i)
                            sum += LookupSwitch(c_shuffleOrder[i]);
                }
            }
        }

        // sparse shuffle sum tests
        {
            printf("\nSparse Shuffle Sum Timings:\n");

            {
                SBlockTimerAggregator timerAgg("  std::vector   ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_sparseNumValues; ++i)
                            sum += stdvector[c_sparseShuffleOrder[i]];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  std::array    ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_sparseNumValues; ++i)
                            sum += stdarray[c_sparseShuffleOrder[i]];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  c array       ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_sparseNumValues; ++i)
                            sum += carray[c_sparseShuffleOrder[i]];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  dynamic memory");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_sparseNumValues; ++i)
                            sum += dynamicmemory[c_sparseShuffleOrder[i]];
                }
            }
            {
                SBlockTimerAggregator timerAgg("  Switch        ");
                for (size_t sample = 0; sample < c_testSamples; ++sample)
                {
                    SBlockTimer timer(timerAgg);
                    for (size_t times = 0; times < c_testRepeatCount; ++times)
                        for (size_t i = 0; i < c_sparseNumValues; ++i)
                            sum += SparseLookupSwitch(c_sparseShuffleOrder[i]);
                }
            }
        }

        printf("\n");
        system("pause");
        printf("Sum = %i\n", sum); // to keep sum, and all the things that calculate it from getting optimized away!
    #endif

    return 0;
}

/*

TODO:
* report variance
* redo timings!
! update blog post
 * now report variances
 * wasn't using sparse lookup function


BLOG: Is Code Faster than Data? switch vs array
Question: how does array access speed compare to switch statement speed?
 * x86/x64
 * sequential, random, sparse

Testing Details
 * vs 2013 community edition
 * 5000 random uint32's
 * c style array
 * std::array
 * std::vector
 * dynamic allocated memory
 * Switch statement function to look up a value by index
 * Go through each array 10000 times
 * Sequential Sum:
  * add up all the values in each array for the arrays
  * call the switch function for each index and sum it up
 * Shuffle Sum:
  * Same as sequential sum, but the indices are accessed in a shuffled order
 * Sparse Sum:
  * same as Shuffle sum, but only doing first 1000 items of the shuffle
  * Uses a speciailized switch function which has only those 1000 items in it.  Simulating unused values getting stripped out.

! link to code, in github!

Results:
* switch statement is A LOT slower than memory!
 * makes sense when you look at asm.
  1) Overhead of function call
  2) we compare the switch value against max to see if it's in range. (can avoid this with __assume(0) apparently)
  3) we then calculate our jump, possibly with an extra layer of indirection.
  4) we jump to that location and return a constant.
 * VS array access
  1) calculate memory location based on index.
  2) return value at index
 What if we inline the switch statement function
 * Don't know.. it made the compiler go into "Generating Code" for a long time, never saw it finish!

Follow up questions:
 * How does switch statement speed compare to hash tables?
 * If hash tables are faster than switch statements, the "hash assisted string to enum" code I made is probably slower than a hash table
  * The compile time assurance of no hash collisions is really nice though.
  ? can we get the benefits of both?



OLD TIMINGS WITHOUT VARIANCE, AND WRONG SWITCH FUNCTION CALLED IN SPARSE CASE:

Win32 Debug
    Sequential Sum Timings:
      c array: 108.48 ms
      std::array: 1135.56 ms
      std::vector: 2160.68 ms
      dynamic memory: 109.56 ms
      Switch: 1687.78 ms

    Shuffle Sum Timings:
      c array: 109.09 ms
      std::array: 1093.76 ms
      std::vector: 2156.12 ms
      dynamic memory: 102.07 ms
      Switch: 1749.74 ms

    Sparse Shuffle Sum Timings:
      c array: 20.01 ms
      std::array: 218.15 ms
      std::vector: 433.83 ms
      dynamic memory: 21.03 ms
      Switch: 294.71 ms

Win32 Release:
    Sequential Sum Timings:
      c array: 4.49 ms
      std::array: 3.50 ms
      std::vector: 4.50 ms
      dynamic memory: 4.50 ms
      Switch: 453.49 ms

    Shuffle Sum Timings:
      c array: 21.03 ms
      std::array: 21.52 ms
      std::vector: 22.00 ms
      dynamic memory: 22.02 ms
      Switch: 487.85 ms

    Sparse Shuffle Sum Timings:
      c array: 4.00 ms
      std::array: 4.00 ms
      std::vector: 4.00 ms
      dynamic memory: 4.50 ms
      Switch: 64.05 ms


x64 Debug:
    Sequential Sum Timings:
      c array: 110.08 ms
      std::array: 501.36 ms
      std::vector: 576.91 ms
      dynamic memory: 108.09 ms
      Switch: 1186.33 ms

    Shuffle Sum Timings:
      c array: 107.08 ms
      std::array: 491.34 ms
      std::vector: 569.40 ms
      dynamic memory: 101.57 ms
      Switch: 1264.53 ms

    Sparse Shuffle Sum Timings:
      c array: 21.52 ms
      std::array: 99.07 ms
      std::vector: 116.08 ms
      dynamic memory: 20.51 ms
      Switch: 193.64 ms

x64 Release:
    Sequential Sum Timings:
      c array: 3.99 ms
      std::array: 3.51 ms
      std::vector: 4.50 ms
      dynamic memory: 3.51 ms
      Switch: 490.96 ms

    Shuffle Sum Timings:
      c array: 23.52 ms
      std::array: 18.53 ms
      std::vector: 17.51 ms
      dynamic memory: 18.02 ms
      Switch: 524.11 ms

    Sparse Shuffle Sum Timings:
      c array: 3.00 ms
      std::array: 3.02 ms
      std::vector: 3.50 ms
      dynamic memory: 3.00 ms
      Switch: 65.56 ms

*/