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
void TestPDFOnly (const char* fileName, const PDF_LAMBDA& PDF)
{
    // make the CDF lookup table by sampling the PDF
    // NOTE: we could integrate the buckets by averaging multiple samples instead of just the 1. This bucket integration is pretty low tech and low quality.
    std::array<float, LOOKUP_TABLE_SIZE> CDFLookupTable;
    float value = 0.0f;
    for (size_t i = 0; i < LOOKUP_TABLE_SIZE; ++i)
    {
        float x = float(i) / float(LOOKUP_TABLE_SIZE - 1); // The -1 is so we cover the full range from 0% to 100%
        value += PDF(x);
        CDFLookupTable[i] = value;
    }

    // normalize the CDF - make sure we span the probability range 0 to 1.
    for (float& f : CDFLookupTable)
        f /= value;

    // make our LUT based inverse CDF
    // We will binary search over the y's (which are sorted smallest to largest) looking for the x, which is implied by the index.
    // I'm sure there's a better & more clever lookup table setup for this situation but this should give you an idea of the technique
    auto inverseCDF = [&CDFLookupTable] (float y) {

        // there is an implicit entry of "0%" at index -1
        if (y < CDFLookupTable[0])
        {
            float t = y / CDFLookupTable[0];
            return t / float(LOOKUP_TABLE_SIZE);
        }

        // get the lower bound in the lut using a binary search
        auto it = std::lower_bound(CDFLookupTable.begin(), CDFLookupTable.end(), y);

        // figure out where we are at in the table
        size_t index = it - CDFLookupTable.begin();

        // Linearly interpolate between the values
        // NOTE: could do other interpolation methods, like perhaps cubic (https://blog.demofox.org/2015/08/08/cubic-hermite-interpolation/)
        float t = (y - CDFLookupTable[index - 1]) / (CDFLookupTable[index] - CDFLookupTable[index - 1]);
        float fractionalIndex = float(index) + t;
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
        TestPDFOnly<100000000, 100, 3>("test3_100m_3.csv", PDF);
        TestPDFOnly<100000000, 100, 5>("test3_100m_5.csv", PDF);
        TestPDFOnly<100000000, 100, 10>("test3_100m_10.csv", PDF);
        TestPDFOnly<100000000, 100, 25>("test3_100m_25.csv", PDF);
        TestPDFOnly<100000000, 100, 100>("test3_100m_100.csv", PDF);
    }

    return 0;
}
