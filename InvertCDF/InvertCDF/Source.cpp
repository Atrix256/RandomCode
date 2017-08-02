#include <stdio.h>
#include <random>

int main (int argc, char **argv)
{
    FILE *file = fopen("blah.csv", "w+t");

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (size_t i = 0; i < 1000; ++i)
    {
        fprintf(file, "\"%f\"\n", std::sqrt(dist(rng)));
    }

    fclose(file);

    return 0;
}