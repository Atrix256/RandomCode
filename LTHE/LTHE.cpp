#include "LTHE.h"
#include <stdio.h>
#include <random>

//=================================================================================
static float RandomFloat ()
{
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

//=================================================================================
static size_t RandomSizeT (size_t min, size_t max)
{
	static std::random_device rd;
	static std::mt19937 mt(rd());
	std::uniform_int<size_t> dist(min, max);  // inclusive
	return dist(mt);
}

namespace LTHE
{
	bool Encrypt (std::vector<float> values, size_t listSize, const char* fileName, std::vector<size_t>& keys)
	{
		// Make sure we have a list that is at least as long as the values we want to encrypt
		if (values.size() > listSize)
		{
			printf("ERROR in " __FUNCTION__ "(): values.size() > listSize!\n");
			return false;
		}

		// Generate keys
		// NOTE: This could be done with format preserving encryption or
		// some other method, to make it not roll and check, and also more secure
		// random.
		keys.clear();
		for (size_t i = 0, c = values.size(); i < c; ++i)
		{
			size_t newKey;
			do
			{
				newKey = RandomSizeT(0, listSize-1);
			} while (std::find(keys.begin(), keys.end(), newKey) != keys.end());
			keys.push_back(newKey);
		}

		return true;
	}
}