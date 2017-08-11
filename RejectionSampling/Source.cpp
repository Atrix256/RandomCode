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
    std::uniform_int_distribution<size_t> dist(0, ACTUAL_DICE_SIDES-1);

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
    fprintf(file, "Actual Count, Expected Count, , %0.2f samples needed per roll on average.\n", (float(NUM_TEST_SAMPLES) + float(rejectedSamples)) / float(NUM_TEST_SAMPLES));
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
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> failedTestCounts = { 0 };
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
                failedTestCounts[std::min(bin, NUM_HISTOGRAM_BUCKETS - 1)]++;
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
            float(failedTestCounts[i])
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
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> failedTestCounts = { 0 };
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
                failedTestCounts[std::min(bin, NUM_HISTOGRAM_BUCKETS - 1)]++;
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
            float(failedTestCounts[i])
        );
    }
    fclose(file);
}

template <size_t NUM_TEST_SAMPLES, size_t NUM_HISTOGRAM_BUCKETS, typename PDF_F_LAMBDA, typename PDF_G_LAMBDA, typename INVERSE_CDF_G_LAMBDA>
void TestPDFToPDF (const char* fileName, const PDF_F_LAMBDA& PDF_F, const PDF_G_LAMBDA& PDF_G, float M, const INVERSE_CDF_G_LAMBDA& Inverse_CDF_G, float rngRange)
{
    // We generate a sample from PDF F by generating a sample from PDF G, and accepting it with probability PDF_F(x)/(M*PDF_G(x))

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
 
    // generate the histogram
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> histogram = { 0 };
    std::array<size_t, NUM_HISTOGRAM_BUCKETS> failedTestCounts = { 0 };
    size_t rejectedSamples = 0;
    for (size_t i = 0; i < NUM_TEST_SAMPLES; ++i)
    {
        // generate random points until we have one that's accepted
        // NOTE: this takes an unknown number of iterations, and technically may NEVER finish.
        float sampleG = 0.0f;
        bool validPoint = false;
        while (!validPoint)
        {
            // Generate a sample from the soure PDF G
            sampleG = Inverse_CDF_G(dist(rng));

            // calculate the ratio of how likely we are to accept this sample
            float acceptChance = PDF_F(sampleG) / (M * PDF_G(sampleG));

            // see if we should accept it
            validPoint = dist(rng) <= acceptChance;

            // track number of failed tests per histogram bucket
            if (!validPoint)
            {
                size_t bin = (size_t)std::floor(sampleG * float(NUM_HISTOGRAM_BUCKETS) / rngRange);
                failedTestCounts[std::min(bin, NUM_HISTOGRAM_BUCKETS - 1)]++;
                ++rejectedSamples;
            }
        }

        // increment the correct bin in the histogram
        size_t bin = (size_t)std::floor(sampleG * float(NUM_HISTOGRAM_BUCKETS) / rngRange);
        histogram[std::min(bin, NUM_HISTOGRAM_BUCKETS - 1)]++;
    }
 
    // write the histogram and pdf sample to a csv
    FILE *file = fopen(fileName, "w+t");
    fprintf(file, "PDF F,PDF G,Scaled PDF G,Simulated PDF,Failed Tests,%0.2f samples needed per value on average.\n", (float(NUM_TEST_SAMPLES) + float(rejectedSamples)) / float(NUM_TEST_SAMPLES));
    for (size_t i = 0; i < NUM_HISTOGRAM_BUCKETS; ++i)
    {
        float x = (float(i) + 0.5f) * rngRange / float(NUM_HISTOGRAM_BUCKETS);
        
        fprintf(file, "%f,%f,%f,%f,%f\n",
            PDF_F(x),
            PDF_G(x),
            PDF_G(x)*M,
            NUM_HISTOGRAM_BUCKETS * float(histogram[i]) / (float(NUM_TEST_SAMPLES)*rngRange),
            float(failedTestCounts[i])
        );
    }
    fclose(file);
}
 
int main(int argc, char **argv)
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
        Test<10000000, 100>("test3_10m_1_15.csv", 1.15f, PDF);
        Test<10000000, 100>("test3_10m_1_5.csv", 1.5f, PDF);
        Test<10000000, 100>("test3_10m_2_8.csv", 2.8f, PDF);
    }

    // function (not PDF, Doesn't integrate to 1!) y=(x^3-10x^2+5x+11), simulated with a scaled up uniform distribution
    {
        auto PDF = [](float x) {return (x*x*x - 10.0f*x*x + 5.0f*x + 11.0f); };
        TestNotPDF<10000000, 100>("test4_10m_12_5.csv", 12.5f, 10.417f, PDF);
    }

    // Generate samples from PDF F using samples from PDF G.  random numbers are from 0 to 30.
    // F PDF = gaussian distribution, mean 15, std dev of 5.  Truncated to +/- 3 stddeviations.
    // G PDF = x*0.002222
    // G CDF = 0.001111 * x^2
    // G inverted CDF = (1000 * sqrt(x)) / sqrt(1111)
    // M = 3
    {
        // gaussian PDF F
        const float mean = 15.0f;
        const float stddev = 5.0f;
        auto PDF_F = [=] (float x) -> float
        {
            return (1.0f / (stddev * sqrt(2.0f * (float)std::_Pi))) * std::exp(-0.5f * pow((x - mean) / stddev, 2.0f));
        };

        // PDF G
        auto PDF_G = [](float x) -> float
        {
            return x * 0.002222f;
        };

        // Inverse CDF of G
        auto Inverse_CDF_G = [] (float x) -> float
        {
            return 1000.0f * std::sqrtf(x) / std::sqrtf(1111.0f);
        };

        TestPDFToPDF<20000, 100>("test5.csv", PDF_F, PDF_G, 3.0f, Inverse_CDF_G, 30.0f);
    }

    return 0;
}
