#include <stdio.h>
#include <random>

int main (int argc, char **argv)
{
    FILE *file = fopen("blah.csv", "w+t");

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, std::nextafter(1.0f, std::numeric_limits<float>::max()));

    for (size_t i = 0; i < 100000; ++i)
    {
        float x = dist(rng);
        float y = std::sqrt(x);
        fprintf(file, "%f,%f\n", x, y);
    }

    fclose(file);

    return 0;
}