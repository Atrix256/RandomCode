#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <random>
#include <array>

template <size_t NUMSAMPLES, size_t NUMBUCKETS, typename PDFLAMBDA, typename INVERSECDFLAMBDA>
void Test (const char* fileName, const PDFLAMBDA& PDF, const INVERSECDFLAMBDA& inverseCDF)
{
    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // generate the histogram
    std::array<size_t, NUMBUCKETS> histogram = { 0 };
    for (size_t i = 0; i < NUMSAMPLES; ++i)
    {
        // put a uniform random number into the inverted CDF to sample the PDF
        float x = dist(rng);
        float y = inverseCDF(x);

        // increment the correct bin on the histogram
        size_t bin = (size_t)std::floor(y * float(NUMBUCKETS));
        histogram[std::min(bin, NUMBUCKETS-1)]++;
    }

    // write the histogram and pdf sample to a csv
    FILE *file = fopen(fileName, "w+t");
    fprintf(file, "PDF, Inverted CDF\n");
    for (size_t i = 0; i < NUMBUCKETS; ++i)
    {
        float x = (float(i) + 0.5f) / float(NUMBUCKETS);
        float pdfSample = PDF(x);
        fprintf(file, "%f,%f\n",
            pdfSample,
            NUMBUCKETS * float(histogram[i]) / float(NUMSAMPLES)
        );
    }
    fclose(file);
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
        auto PDF = [](float x) { return 3.0f * x * x; };
        auto inverseCDF = [](float x) { return std::pow(x, 1.0f / 3.0f); };

        Test<100000, 100>("test2_100k.csv", PDF, inverseCDF);
    }

    return 0;
}

// TODO: why do you need to multiply histogram sample * NUMBUCKETS to make it match the PDF?

