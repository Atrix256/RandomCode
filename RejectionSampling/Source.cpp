#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <random>
#include <array>
#include <unordered_map>
 
template <size_t NUM_TEST_SAMPLES, size_t NUM_HISTOGRAM_BUCKETS, typename PDF_LAMBDA>
void Test (const char* fileName, float maxPDFValue, const PDF_LAMBDA& PDF)
{
    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
 
    // generate the histogram
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> histogram = { 0 };
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> testCounts = { 0 };
    for (size_t i = 0; i < NUM_TEST_SAMPLES; ++i)
    {
        // Generate a sample from the PDF by generating a random 2d point.
        // If the y axis of the value is <= the value returned by PDF(x), accept it, else reject it.
        // NOTE: this takes an unknown number of iterations, and technically may NEVER finish.
        float pointX = 0.0f;
        float pointY = 0.0f;
        bool validPoint = false;
        while (!validPoint)
        {
            pointX = dist(rng);
            pointY = dist(rng) * maxPDFValue;
            float pdfValue = PDF(pointX);
            validPoint = (pointY <= pdfValue);

            // track number of failed tests per histogram bucket
            if (!validPoint)
            {
                size_t bin = (size_t)std::floor(pointX * float(NUM_HISTOGRAM_BUCKETS));
                testCounts[std::min(bin, NUM_HISTOGRAM_BUCKETS - 1)]++;
            }
        }
 
        // increment the correct bin in the histogram
        size_t bin = (size_t)std::floor(pointX * float(NUM_HISTOGRAM_BUCKETS));
        histogram[std::min(bin, NUM_HISTOGRAM_BUCKETS -1)]++;
    }
 
    // write the histogram and pdf sample to a csv
    FILE *file = fopen(fileName, "w+t");
    fprintf(file, "PDF, Inverted CDF, Failed Tests\n");
    for (size_t i = 0; i < NUM_HISTOGRAM_BUCKETS; ++i)
    {
        float x = (float(i) + 0.5f) / float(NUM_HISTOGRAM_BUCKETS);
        float pdfSample = PDF(x);
        fprintf(file, "%f,%f,%f\n",
            pdfSample,
            NUM_HISTOGRAM_BUCKETS * float(histogram[i]) / float(NUM_TEST_SAMPLES),
            float(testCounts[i])
        );
    }
    fclose(file);
}
 
int main (int argc, char **argv)
{
    // PDF: y=2x
    {
        auto PDF = [](float x) { return 2.0f * x; };

        Test<1000, 100>("test1_1k.csv", 2.0f, PDF);
        Test<100000, 100>("test1_100k.csv", 2.0f, PDF);
        Test<1000000, 100>("test1_1m.csv", 2.0f, PDF);
    }

    /*
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
        TestPDFOnly<100000000, 100, 25>("test3_100m_100.csv", PDF);
    }
    */
 
    return 0;
}
/*

TODO:
* Simple version: uniform random numbers
* More complex version: use a different distribution to generate a third distribution!  maybe use gaussian to generate an exponential or something

ARS:
Idea: when rejection sampling you want to sample something that has very little wasted space so you throw away fewer samples.
Make a PDF out of line segments.
When a sample fails, you know where the PDF needs to fit, so you shrink it down.
Somehow limited to convex (upside down U) PDFs.
Works in log space, somehow better.

*/