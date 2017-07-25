#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <random>
#include <stdint.h>
#include <stdio.h>
#include <limits>

const size_t c_maxNumSamples = 1000000;
const char* c_fileName = "results.csv";

template <size_t DiceRange, size_t DiceCount, size_t NumBuckets>
void DumpBucketCountsAddRandomNumbers (size_t numSamples, const std::array<size_t, NumBuckets>& bucketCounts)
{
    // open file for append if we can
    FILE* file = fopen(c_fileName, "a+t");
    if (!file)
        return;

    // write the info
    float mean = float(DiceCount) * float(DiceRange - 1.0f) / 2.0f;
    float variance = float(DiceCount) * (DiceRange * DiceRange) / 12.0f;
    if (numSamples == 1)
    {
        fprintf(file, "\"%zu random numbers [0,%zu) added together (sum %zud%zu). %zu buckets.  Mean = %0.2f.  Variance = %0.2f.  StdDev = %0.2f.\"\n", DiceCount, DiceRange, DiceCount, DiceRange, NumBuckets, mean, variance, std::sqrt(variance));
        fprintf(file, "\"\"");
        for (size_t i = 0; i < NumBuckets; ++i)
            fprintf(file, ",\"%zu\"", i);
        fprintf(file, "\n");
    }
    fprintf(file, "\"%zu samples\",", numSamples);

    // report the samples
    for (size_t count : bucketCounts)
        fprintf(file, "\"%zu\",", count);

    fprintf(file, "\"\"\n");
    if (numSamples == c_maxNumSamples)
        fprintf(file, "\n");

    // close file
    fclose(file);
}

template <size_t DiceSides, size_t DiceCount>
void AddRandomNumbersTest ()
{
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<size_t> dist(size_t(0), DiceSides - 1);

    std::array<size_t, (DiceSides - 1) * DiceCount + 1> bucketCounts = { 0 };

    size_t nextDump = 1;
    for (size_t i = 0; i < c_maxNumSamples; ++i)
    {
        size_t sum = 0;
        for (size_t j = 0; j < DiceCount; ++j)
            sum += dist(rng);

        bucketCounts[sum]++;

        if (i + 1 == nextDump)
        {
            DumpBucketCountsAddRandomNumbers<DiceSides, DiceCount>(nextDump, bucketCounts);
            nextDump *= 10;
        }
    }
}

template <size_t NumBuckets>
void DumpBucketCountsCountBits (size_t numSamples, const std::array<size_t, NumBuckets>& bucketCounts)
{
    // open file for append if we can
    FILE* file = fopen(c_fileName, "a+t");
    if (!file)
        return;

    // write the info
    float mean = float(NumBuckets-1) * 1.0f / 2.0f;
    float variance = float(NumBuckets-1) * 3.0f / 12.0f;
    if (numSamples == 1)
    {
        fprintf(file, "\"%zu random bits (coin flips) added together. %zu buckets.  Mean = %0.2f.  Variance = %0.2f.  StdDev = %0.2f.\"\n", NumBuckets - 1, NumBuckets, mean, variance, std::sqrt(variance));
        fprintf(file, "\"\"");
        for (size_t i = 0; i < NumBuckets; ++i)
            fprintf(file, ",\"%zu\"", i);
        fprintf(file, "\n");
    }
    fprintf(file, "\"%zu samples\",", numSamples);

    // report the samples
    for (size_t count : bucketCounts)
        fprintf(file, "\"%zu\",", count);

    fprintf(file, "\"\"\n");
    if (numSamples == c_maxNumSamples)
        fprintf(file, "\n");

    // close file
    fclose(file);
}

template <size_t NumBits> // aka NumCoinFlips!
void CountBitsTest ()
{

    size_t maxValue = 0;
    for (size_t i = 0; i < NumBits; ++i)
        maxValue = (maxValue << 1) | 1;

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<size_t> dist(0, maxValue);

    std::array<size_t, NumBits + 1> bucketCounts = { 0 };

    size_t nextDump = 1;
    for (size_t i = 0; i < c_maxNumSamples; ++i)
    {
        size_t sum = 0;
        size_t number = dist(rng);
        while (number)
        {
            if (number & 1)
                ++sum;
            number = number >> 1;
        }

        bucketCounts[sum]++;

        if (i + 1 == nextDump)
        {
            DumpBucketCountsCountBits(nextDump, bucketCounts);
            nextDump *= 10;
        }
    }
}

float GenerateNormalRandomNumber (float mean, float variance)
{
    static std::mt19937 rng;
    static std::uniform_int_distribution<uint64_t> dist(0, (uint64_t)-1);

    static bool seeded = false;
    if (!seeded)
    {
        seeded = true;
        rng.seed(std::random_device()());
    }

    // generate our normal distributed random number from 0 to 65.
    // 
    float sum = 0.0f;
    uint64_t number = dist(rng);
    while (number)
    {
        if (number & 1)
            sum += 1.0f;
        number = number >> 1;
    }

    // convert from: mean 32, variance 16, stddev 4
    // to: mean 0, variance 1, stddev 1
    float ret = sum;
    ret -= 32.0f;
    ret /= 4.0f;

    // convert to the specified mean and variance
    ret *= std::sqrt(variance);
    ret += mean;
    return ret;
}

void VerifyGenerateNormalRandomNumber (float mean, float variance)
{
    // open file for append if we can
    FILE* file = fopen(c_fileName, "a+t");
    if (!file)
        return;

    // write info
    fprintf(file, "\"Normal Distributed Random Numbers. mean = %0.2f.  variance = %0.2f.  stddev = %0.2f\"\n", mean, variance, std::sqrt(variance));

    // write some random numbers
    fprintf(file, "\"100 numbers\"");
    for (size_t i = 0; i < 100; ++i)
        fprintf(file, ",\"%f\"", GenerateNormalRandomNumber(mean, variance));
    fprintf(file, "\n\n");

    // close file
    fclose(file);
}

int main (int argc, char **argv)
{
    // clear out the file
    FILE* file = fopen(c_fileName, "w+t");
    if (file)
        fclose(file);

    // coin flips
    {
        // flip a fair coin 
        AddRandomNumbersTest<2, 1>();

        // flip two coins and sum them
        AddRandomNumbersTest<2, 2>();

        // sum 3 coin flips
        AddRandomNumbersTest<2, 3>();

        // sum 100 coin flips
        AddRandomNumbersTest<2, 100>();
    }

    // dice rolls
    {
        // roll a 4 sided die
        AddRandomNumbersTest<4, 1>();

        // sum two 4 sided dice
        AddRandomNumbersTest<4, 2>();

        // sum three 4 sided dice
        AddRandomNumbersTest<4, 3>();

        // sum one hundred 4 sided dice
        AddRandomNumbersTest<4, 100>();

        // sum one hundred 6 sided dice
        AddRandomNumbersTest<6, 100>();
    }

    CountBitsTest<8>();
    CountBitsTest<16>();
    CountBitsTest<32>();
    CountBitsTest<64>();

    VerifyGenerateNormalRandomNumber(0.0f, 20.0f);

    VerifyGenerateNormalRandomNumber(0.0f, 10.0f);

    VerifyGenerateNormalRandomNumber(5.0f, 10.0f);

    return 0;
}


/*

BLOG:

* thank alan hickman

? which tests specifically would be good to do and show for blog post?
 ? show difference in increasing dice side vs dice count?
 * mean is (n-1)/2.  Or if 1 based: (n+1)/2.
 * variance is (n*n-1)/12.
 * those are multiplied by the number of dice summed.

* two names: normal / gaussian distribution

* mention std function for gaussian distribution
 * and an: http://www.design.caltech.edu/erik/Misc/Gaussian.html

* Also, make your code / explain how to make distributions with different means and variance squared

* converting one distribution to another: https://stats.stackexchange.com/questions/43114/how-to-convert-to-gaussian-distribution-with-given-mean-and-standard-deviation/43117#43117

! can verify GenerateNormalRandomNumber() function by having excel calculate average and stddeviation

john cook links that inspired this:
* https://www.johndcook.com/blog/2015/03/09/why-isnt-everything-normally-distributed/
* https://www.johndcook.com/blog/2009/09/29/achievement-is-log-normal/


* irrelevant?
* i think your normalization is still wrong. wikipedia shows regular old std dev with a peak of 0.4.  Yours isn't / doesn't. Why?
 * you aren't taking into account the width of your bins when normalizing it to integrate it to 1.0?
 * https://math.stackexchange.com/questions/468541/normalizing-a-gaussian-distribution
 * +/- 3 stddeviations is 99.7%.  Does it ever reach 100%? If not, maybe saying the graph has width 6 overall is enough to make it normalize (to integrate to 1.0) correctly?
 * you need to understand this and explain it in the blog post


*/
