#include <stdio.h>
#include <random>

static const float c_goldenRatioConjugate = 0.61803398875f;

int main (int argc, char**argv)
{
    static const int c_numItems = 100;
    static const int c_numSamples = 10000;

    // start at a random location in the list
    std::random_device rd("dev/random");
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0, 1);
    float itemFloat = dist(rng);

    // do the golden ratio shuffle
    FILE* reportFile = nullptr;
    FILE* sequenceFile = nullptr;
    fopen_s(&reportFile, "report.csv", "w+t");
    fprintf(reportFile, "\"index\",\"min\",\"max\",\"diff\"\n");
    fopen_s(&sequenceFile, "sequence.csv", "w+t");
    fprintf(sequenceFile, "\"index\",\"itemFloat\",\"item\"\n");
    std::vector<int> itemCounts(c_numItems, 0);
    for (int sampleIndex = 0; sampleIndex < c_numSamples; ++sampleIndex)
    {
        itemFloat = fmodf(itemFloat + c_goldenRatioConjugate, 1.0f);
        int item = int(itemFloat * float(c_numItems));

        itemCounts[item]++;

        int min = itemCounts[0];
        int max = itemCounts[0];
        for (int itemCount : itemCounts)
        {
            min = std::min(min, itemCount);
            max = std::max(max, itemCount);
        }

        fprintf(reportFile, "\"%i\",\"%i\",\"%i\",\"%i\"\n", sampleIndex, min, max, max-min);
        fprintf(sequenceFile, "\"%i\",\"%f\",\"%i\"\n", sampleIndex, itemFloat, item);
    }
    fclose(reportFile);
    fclose(sequenceFile);

    FILE* histogramFile = nullptr;
    fopen_s(&histogramFile, "histogram.csv", "w+t");
    for (int index = 0; index < c_numItems; ++index)
        fprintf(histogramFile, "\"%i\",\"%i\"\n", index, itemCounts[index]);
    fclose(histogramFile);

    return 0;
}
