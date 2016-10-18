#pragma once
#include <vector>

namespace LTHE
{
    typedef float(*TTransformationFN)(float);

    bool Encrypt (std::vector<float> values, size_t listSize, const char* fileName, std::vector<size_t>& keys);

    bool TransformHomomorphically (const char* fileName, TTransformationFN function);

    bool Decrypt (const char* fileName, std::vector<float>& values, std::vector<size_t>& keys);
}