#pragma once
#include <vector>

namespace LTHE
{
	bool Encrypt (std::vector<float> values, size_t listSize, const char* fileName, std::vector<size_t>& keys);
}