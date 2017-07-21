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
    if (numSamples == 1)
        fprintf(file, "\"%zu random numbers [0,%zu) added together (sum %zud%zu). %zu buckets.\"\n", DiceCount, DiceRange, DiceCount, DiceRange, NumBuckets);
    fprintf(file, "\"%zu samples\",", numSamples);

    // report the samples such that they integrate to 1.0
    for (size_t count : bucketCounts)
        fprintf(file, "\"%f\",", float(count) / float(numSamples));

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
    if (numSamples == 1)
        fprintf(file, "\"%zu random bits (coin flips) added together. %zu buckets.\"\n", NumBuckets - 1, NumBuckets);
    fprintf(file, "\"%zu samples\",", numSamples);

    // report the samples such that they integrate to 1.0
    for (size_t count : bucketCounts)
        fprintf(file, "\"%f\",", float(count) / float(numSamples));

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

int main (int argc, char **argv)
{
    // clear out the file
    FILE* file = fopen(c_fileName, "w+t");
    if (file)
        fclose(file);

    // Do some tests
    AddRandomNumbersTest<2, 1>();
    AddRandomNumbersTest<2, 2>();
    AddRandomNumbersTest<2, 5>();
    AddRandomNumbersTest<2, 10>();
    AddRandomNumbersTest<2, 100>();

    AddRandomNumbersTest<5, 5>();
    AddRandomNumbersTest<10, 10>();

    CountBitsTest<8>();
    CountBitsTest<16>();
    CountBitsTest<32>();
    CountBitsTest<64>();


    return 0;
}


/*

* grab notes from email and put in here.

! make a function for gaussian distribution. RNG a uint64, count bits, divide by 64?
 * should you also prove it's right?

? which tests specifically would be good to do and show for blog post?
 
* i think your normalization is still wrong. wikipedia shows regular old std dev with a peak of 0.4.  Yours isn't / doesn't. Why?
 * you aren't taking into account the width of your bins when normalizing it to integrate it to 1.0?
 * https://math.stackexchange.com/questions/468541/normalizing-a-gaussian-distribution
 * +/- 3 stddeviations is 99.7%.  Does it ever reach 100%? If not, maybe saying the graph has width 6 overall is enough to make it normalize (to integrate to 1.0) correctly?
 * you need to understand this and explain it in the blog post


? better name for variables than Numbers, NumSamples, SampleCount?

* Also the thing about counting number of bits in a random number to make gaussian distribution.
 * They are related because you are adding random 1 bit numbers!

* With both methods: there are that many ways to make that total.

* Can we generate log normal distribution?

* two dimensional distribution, like gaussian kernel?
 * could do with a tensor product but explain how it relates to "there are that manyt ways to make that total"?

BLOG:
* two names: normal / gaussian distribution

*/
