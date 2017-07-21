#define _CRT_SECURE_NO_WARNINGS

#include <array>
#include <random>
#include <stdint.h>
#include <stdio.h>

const size_t c_numTests = 1000000;
const char* c_fileName = "results.csv";

template <size_t Numbers, size_t NumSamples, size_t NumBuckets>
void DumpBucketCounts (size_t numSamples, const std::array<size_t, NumBuckets>& bucketCounts)
{
    // open file for append if we can
    FILE* file = fopen(c_fileName, "a+t");
    if (!file)
        return;

    // write the info
    if (numSamples == 1)
        fprintf(file, "%zu random numbers [0,%zu) added together. %zu buckets.\n", NumSamples, Numbers, NumBuckets);
    
    fprintf(file, "\"%zu samples\",", numSamples);

    size_t maxBucket = 0;
    for (size_t count : bucketCounts)
        maxBucket = std::max(maxBucket, count);

    for (size_t count : bucketCounts)
        fprintf(file, "\"%f\",", float(count) / float(maxBucket));

    fprintf(file, "\"\"\n");

    if (numSamples == c_numTests)
        fprintf(file, "\n");

    // close file
    fclose(file);
}

template <size_t Numbers, size_t NumSamples>
void AddRandomNumbersTest ()
{
    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<size_t> dist(size_t(0), Numbers-1);

    std::array<size_t, (Numbers-1) * NumSamples + 1> bucketCounts = { 0 };

    size_t nextDump = 1;
    for (size_t i = 0; i < c_numTests; ++i)
    {
        size_t number = 0;
        for (size_t j = 0; j < NumSamples; ++j)
            number += dist(rng);

        bucketCounts[number]++;

        if (i + 1 == nextDump)
        {
            DumpBucketCounts<Numbers, NumSamples>(nextDump, bucketCounts);
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
    AddRandomNumbersTest<2, 2>();
    AddRandomNumbersTest<4, 4>();


    return 0;
}


/*

* i think your normalization is wrong.  Need all data points to add to 1.0, so get the sum and divide everything by that!

* make there be a heading row, and then just the various number of sample rows, and then a few newlines.

* print a floating point value, normalized to be between 0 and 1?

? better name for columns that Numbers, NumSamples, SampleCount?

* Add a bunch of random numbers, bucket counts for each possible value.

* Also the thing about counting number of bits in a random number to make gaussian distribution.
 * They are related because you are adding random 1 bit numbers!

* With both methods: there are that many ways to make that total.

* Can we generate log normal distribution?

* two dimensional?

BLOG:
* two names: normal / gaussian distribution

*/
