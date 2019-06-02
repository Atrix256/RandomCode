#include <random>
#include <stdio.h>
#include <array>

static const size_t c_maxValue = 255;
static const size_t c_numSamples = 1000000;

int main(int argc, char** argv)
{
    std::random_device rd;
    std::seed_seq fullSeed{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
    std::mt19937 rng(fullSeed);

    std::uniform_int_distribution<size_t> dist(0, c_maxValue);
    std::array<size_t, c_maxValue + 1> counts{};
    std::array<size_t, c_maxValue + 1> countsAverage2{};  // same shape as "add" but scaled down so we don't need a 2x as large histogram
    std::array<size_t, c_maxValue + 1> countsAverage3{};
    std::array<size_t, c_maxValue + 1> countsAverage4{};
    std::array<size_t, c_maxValue + 1> countsMax2{};
    std::array<size_t, c_maxValue + 1> countsMax3{};
    std::array<size_t, c_maxValue + 1> countsMax4{};
    std::array<size_t, c_maxValue + 1> countsMin2{};
    std::array<size_t, c_maxValue + 1> countsMin3{};
    std::array<size_t, c_maxValue + 1> countsMin4{};

    // generate data
    for (size_t index = 0; index < c_numSamples; ++index)
    {
        size_t value1 = dist(rng);
        size_t value2 = dist(rng);
        size_t value3 = dist(rng);
        size_t value4 = dist(rng);

        counts[value1]++;

        countsAverage2[(value1 + value2) / 2]++;
        countsAverage3[(value1 + value2 + value3) / 3]++;
        countsAverage4[(value1 + value2 + value3 + value4) / 4]++;

        countsMax2[std::max(value1, value2)]++;
        countsMax3[std::max(value1, std::max(value2, value3))]++;
        countsMax4[std::max(std::max(value1, value2), std::max(value3, value4))]++;

        countsMin2[std::min(value1, value2)]++;
        countsMin3[std::min(value1, std::min(value2, value3))]++;
        countsMin4[std::min(std::min(value1, value2), std::min(value3, value4))]++;
    }

    // write histograms
    FILE *file = nullptr;
    fopen_s(&file, "histograms.csv", "w+t");

    fprintf(file, "\"Index\",\"Count1\",\"Average2\",\"Average3\",\"Average4\",\"Max2\",\"Max3\",\"Max4\",\"Min2\",\"Min3\",\"Min4\",\"y=x\",\"y=x^2\",\"y=x^3\"\n");

    for (size_t index = 0; index < c_maxValue + 1; ++index)
    {
        fprintf(file, "\"%zu\",", index);
        fprintf(file, "\"%zu\",", counts[index]);
        fprintf(file, "\"%zu\",", countsAverage2[index]);
        fprintf(file, "\"%zu\",", countsAverage3[index]);
        fprintf(file, "\"%zu\",", countsAverage4[index]);
        fprintf(file, "\"%zu\",", countsMax2[index]);
        fprintf(file, "\"%zu\",", countsMax3[index]);
        fprintf(file, "\"%zu\",", countsMax4[index]);
        fprintf(file, "\"%zu\",", countsMin2[index]);
        fprintf(file, "\"%zu\",", countsMin3[index]);
        fprintf(file, "\"%zu\",", countsMin4[index]);

        // make the PDFs be the same scale as the histogram counts.
        float x = float(index) / float(c_maxValue);
        fprintf(file, "\"%zu\",", size_t(float(c_numSamples)*x * 2.0f / float(c_maxValue)));
        fprintf(file, "\"%zu\",", size_t(float(c_numSamples)*x*x * 3.0f / float(c_maxValue)));
        fprintf(file, "\"%zu\"\n", size_t(float(c_numSamples)*x*x*x * 4.0f / float(c_maxValue)));
    }

    fclose(file);
}