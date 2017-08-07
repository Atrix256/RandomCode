#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <random>
#include <array>
#include <unordered_map>

template <size_t NUM_TEST_SAMPLES, size_t SIMULATED_DICE_SIDES, size_t ACTUAL_DICE_SIDES>
void TestDice (const char* fileName)
{
    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<size_t> dist(0, ACTUAL_DICE_SIDES);

    // generate the histogram
    std::array<size_t, SIMULATED_DICE_SIDES> histogram = { 0 };
    size_t rejectedSamples = 0;
    for (size_t i = 0; i < NUM_TEST_SAMPLES; ++i)
    {
        size_t roll = dist(rng);
        while (roll >= SIMULATED_DICE_SIDES)
        {
            ++rejectedSamples;
            roll = dist(rng);
        }
        histogram[roll]++;
    }

    // write the histogram and rejected sample count to a csv
    // an extra 0 data point forces the graph to include 0 in the scale. hack to make the data not look noisier than it really is.
    FILE *file = fopen(fileName, "w+t");
    fprintf(file, "Actual Count, Expected Count, Dummy Data, %0.2f samples needed per roll on average.\n", (float(NUM_TEST_SAMPLES) + float(rejectedSamples)) / float(NUM_TEST_SAMPLES));
    for (size_t value : histogram)
        fprintf(file, "%zu,%zu,0\n", value, (size_t)(float(NUM_TEST_SAMPLES) / float(SIMULATED_DICE_SIDES)));
    fclose(file);
}
 
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
    size_t rejectedSamples = 0;
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
                ++rejectedSamples;
            }
        }
 
        // increment the correct bin in the histogram
        size_t bin = (size_t)std::floor(pointX * float(NUM_HISTOGRAM_BUCKETS));
        histogram[std::min(bin, NUM_HISTOGRAM_BUCKETS -1)]++;
    }
 
    // write the histogram and pdf sample to a csv
    FILE *file = fopen(fileName, "w+t");
    fprintf(file, "PDF, Simulated PDF, Generating Function, Failed Tests, %0.2f samples needed per value on average.\n", (float(NUM_TEST_SAMPLES) + float(rejectedSamples)) / float(NUM_TEST_SAMPLES));
    for (size_t i = 0; i < NUM_HISTOGRAM_BUCKETS; ++i)
    {
        float x = (float(i) + 0.5f) / float(NUM_HISTOGRAM_BUCKETS);
        float pdfSample = PDF(x);
        fprintf(file, "%f,%f,%f,%f\n",
            pdfSample,
            NUM_HISTOGRAM_BUCKETS * float(histogram[i]) / float(NUM_TEST_SAMPLES),
            maxPDFValue,
            float(testCounts[i])
        );
    }
    fclose(file);
}

template <size_t NUM_TEST_SAMPLES, size_t NUM_HISTOGRAM_BUCKETS, typename PDF_LAMBDA>
void TestNotPDF (const char* fileName, float maxPDFValue, float normalizationConstant, const PDF_LAMBDA& PDF)
{
    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
 
    // generate the histogram
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> histogram = { 0 };
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> testCounts = { 0 };
    size_t rejectedSamples = 0;
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
                ++rejectedSamples;
            }
        }
 
        // increment the correct bin in the histogram
        size_t bin = (size_t)std::floor(pointX * float(NUM_HISTOGRAM_BUCKETS));
        histogram[std::min(bin, NUM_HISTOGRAM_BUCKETS -1)]++;
    }
 
    // write the histogram and pdf sample to a csv
    FILE *file = fopen(fileName, "w+t");
    fprintf(file, "Function, Simulated PDF, Scaled Simulated PDF, Generating Function, Failed Tests, %0.2f samples needed per value on average.\n", (float(NUM_TEST_SAMPLES) + float(rejectedSamples)) / float(NUM_TEST_SAMPLES));
    for (size_t i = 0; i < NUM_HISTOGRAM_BUCKETS; ++i)
    {
        float x = (float(i) + 0.5f) / float(NUM_HISTOGRAM_BUCKETS);
        float pdfSample = PDF(x);
        fprintf(file, "%f,%f,%f,%f,%f\n",
            pdfSample,
            NUM_HISTOGRAM_BUCKETS * float(histogram[i]) / float(NUM_TEST_SAMPLES),
            NUM_HISTOGRAM_BUCKETS * float(histogram[i]) / float(NUM_TEST_SAMPLES) * normalizationConstant,
            maxPDFValue,
            float(testCounts[i])
        );
    }
    fclose(file);
}
 
int main (int argc, char **argv)
{
    // Dice
    {
        // Simulate a 5 sided dice with a 6 sided dice
        TestDice<10000, 5, 6>("test1_5_6.csv");

        // Simulate a 5 sided dice with a 20 sided dice
        TestDice<10000, 5, 20>("test1_5_20.csv");
    }

    // PDF y=2x, simulated with a uniform distribution
    {
        auto PDF = [](float x) { return 2.0f * x; };

        Test<1000, 100>("test2_1k.csv", 2.0f, PDF);
        Test<100000, 100>("test2_100k.csv", 2.0f, PDF);
        Test<1000000, 100>("test2_1m.csv", 2.0f, PDF);
    }

    // PDF y=(x^3-10x^2+5x+11)/10.417, simulated with a uniform distribution
    {
        auto PDF = [](float x) {return (x*x*x - 10.0f*x*x + 5.0f*x + 11.0f) / (10.417f); };
        Test<10000000, 100>("test3_10m_1_1.csv", 1.1f, PDF);
        Test<10000000, 100>("test3_10m_1_2.csv", 1.2f, PDF);
        Test<10000000, 100>("test3_10m_2_8.csv", 2.8f, PDF);
    }

    // function (not PDF, Doesn't integrate to 1!) y=(x^3-10x^2+5x+11), simulated with a uniform distribution
    {
        auto PDF = [](float x) {return (x*x*x - 10.0f*x*x + 5.0f*x + 11.0f); };
        TestNotPDF<10000000, 100>("test4_10m_12_5.csv", 12.5f, 10.417f, PDF);
    }
 
    // TODO: generate a distribution using a non uniform distribution!

    return 0;
}
/*

TODO:

Blog:
? can you calculate how many rolls should be needed on average for simulating dice? put that in blog post
 * yes. If the thing is a PDF it has area of 1.0.  If the generating distribution has area of X, X is how many samples you need. totally logical and simple
* also note, the PDF just always needs to be greater than the other distribution. If you don't know maximum value thats fine, just choose one that is for sure greater (round up a bit).

ARS:
Idea: when rejection sampling you want to sample something that has very little wasted space so you throw away fewer samples.
Make a PDF out of line segments.
When a sample fails, you know where the PDF needs to fit, so you shrink it down.
Somehow limited to convex (upside down U) PDFs.
Works in log space, somehow better.

*/