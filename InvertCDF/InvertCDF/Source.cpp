#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <random>
#include <array>
#include <unordered_map>

template <size_t NUM_TEST_SAMPLES, size_t NUM_HISTOGRAM_BUCKETS, typename PDF_LAMBDA, typename INVERSE_CDF_LAMBDA>
void Test (const char* fileName, const PDF_LAMBDA& PDF, const INVERSE_CDF_LAMBDA& inverseCDF)
{
    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // generate the histogram
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> histogram = { 0 };
    for (size_t i = 0; i < NUM_TEST_SAMPLES; ++i)
    {
        // put a uniform random number into the inverted CDF to sample the PDF
        float x = dist(rng);
        float y = inverseCDF(x);

        // increment the correct bin on the histogram
        size_t bin = (size_t)std::floor(y * float(NUM_HISTOGRAM_BUCKETS));
        histogram[std::min(bin, NUM_HISTOGRAM_BUCKETS -1)]++;
    }

    // write the histogram and pdf sample to a csv
    FILE *file = fopen(fileName, "w+t");
    fprintf(file, "PDF, Inverted CDF\n");
    for (size_t i = 0; i < NUM_HISTOGRAM_BUCKETS; ++i)
    {
        float x = (float(i) + 0.5f) / float(NUM_HISTOGRAM_BUCKETS);
        float pdfSample = PDF(x);
        fprintf(file, "%f,%f\n",
            pdfSample,
            NUM_HISTOGRAM_BUCKETS * float(histogram[i]) / float(NUM_TEST_SAMPLES)
        );
    }
    fclose(file);
}

template <size_t NUM_TEST_SAMPLES, size_t NUM_HISTOGRAM_BUCKETS, size_t LOOKUP_TABLE_SIZE, typename PDF_LAMBDA>
void Test (const char* fileName, const PDF_LAMBDA& PDF)
{
    // make the PDF lookup table
    std::array<float, LOOKUP_TABLE_SIZE> PDFLookupTable;
    for (size_t i = 0; i < LOOKUP_TABLE_SIZE; ++i)
    {
        float x = float(i) / float(LOOKUP_TABLE_SIZE - 1); // The -1 is so we cover the full range from 0% to 100%
        PDFLookupTable[i] = PDF(x) / float(LOOKUP_TABLE_SIZE);
    }

    // make the CDF lookup table
    std::array<float, LOOKUP_TABLE_SIZE> CDFLookupTable;
    CDFLookupTable[0] = PDFLookupTable[0];
    for (size_t i = 1; i < LOOKUP_TABLE_SIZE; ++i)
        CDFLookupTable[i] = CDFLookupTable[i - 1] + PDFLookupTable[i];

    // make our LUT based inverse CDF
    // We will binary search over the y's (which are sorted smallest to largest) looking for the x, which is implied by the index.
    // I'm sure there's a better & more clever lookup table setup for this situation but this should give you an idea of the technique
    auto inverseCDF = [&CDFLookupTable] (float y) {

        // try to find the value in the LUT
        auto it = std::lower_bound(CDFLookupTable.begin(), CDFLookupTable.end(), y);

        // if we got to the end, it's larger than any values we have, so return 1.0
        if (it == CDFLookupTable.end())
            return 1.0f;

        // figure out where we are at in the table
        size_t index = it - CDFLookupTable.begin();

        // if we are at the beginning, it means the first number is greater than the number we are looking for, so return 0.0
        if (index == 0)
            return 0.0f;

        // else linearly interpolate between the values
        // NOTE: could do other interpolation methods, like perhaps cubic (https://blog.demofox.org/2015/08/08/cubic-hermite-interpolation/)
        float t = (y - CDFLookupTable[index - 1]) / (CDFLookupTable[index] - CDFLookupTable[index - 1]);
        //float x = (index - 1) * t + (index) * (1.0f - t); // Same as line below, but explicit lerp.
        float fractionalIndex = float(index - 1) + t;
        return fractionalIndex / float(LOOKUP_TABLE_SIZE);
    };

    // call the usual function to do the testing
    Test<NUM_TEST_SAMPLES, NUM_HISTOGRAM_BUCKETS>(fileName, PDF, inverseCDF);
}

int main (int argc, char **argv)
{
    // PDF: y=2x
    // inverse CDF: y=sqrt(x)
    {
        auto PDF = [] (float x) { return 2.0f * x; };
        auto inverseCDF = [] (float x) { return std::sqrt(x); };

        Test<1000, 100>("test1_1k.csv", PDF, inverseCDF);
        Test<100000, 100>("test1_100k.csv", PDF, inverseCDF);
        Test<1000000, 100>("test1_1m.csv", PDF, inverseCDF);
    }

    // PDF: y=3x^2
    // inverse CDF: y=cuberoot(x) aka y = pow(x, 1/3)
    {
        auto PDF = [] (float x) { return 3.0f * x * x; };
        auto inverseCDF = [](float x) { return std::pow(x, 1.0f / 3.0f); };

        Test<100000, 100>("test2_100k.csv", PDF, inverseCDF);
    }

    // PDF: y=(x^3-10x^2+5x+11)/10.417
    // Inverse CDF Numerically via a lookup table
    {
        auto PDF = [] (float x) {return (x*x*x - 10.0f*x*x + 5.0f*x + 11.0f) / (10.417f); };
        Test<100000, 100, 50>("test3_100k_50.csv", PDF);
        Test<100000, 100, 100>("test3_100k_100.csv", PDF);
        Test<100000, 100, 1000>("test3_100k_1000.csv", PDF);
    }

    return 0;
}
