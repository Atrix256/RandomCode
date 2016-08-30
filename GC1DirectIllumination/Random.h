#pragma once

#include <stdlib.h>

// from 0 to 1
float RandomFloat ()
{
    // Xorshift random number algorithm invented by George Marsaglia
    static uint32_t rng_state = 0xf2eec0de;
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return float(rng_state) * (1.0f / 4294967296.0f);

    /*
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(mt);
    */
}

float RandomFloat (float min, float max)
{
    return min + (max - min) * RandomFloat();
}