#pragma once

#include <stdint.h>
#include <array>

typedef uint8_t uint8;
typedef std::array<uint8, 3> TPixelBGRU8;
typedef std::array<float, 3> TPixelRGBF32;
const float c_pi = 3.14159265359f;

//=================================================================================
inline float Clamp(float v, float min, float max)
{
    if (v < min)
        return min;
    else if (v > max)
        return max;
    else
        return v;
}

//=================================================================================
// from 0 to 1
float RandomFloat ()
{
    /*
    // Xorshift random number algorithm invented by George Marsaglia
    static uint32_t rng_state = 0xf2eec0de;
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return float(rng_state) * (1.0f / 4294967296.0f);
    */

    // alternately, using a standard c++ prng
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(mt);
}

//=================================================================================
float RandomFloat (float min, float max)
{
    return min + (max - min) * RandomFloat();
}
