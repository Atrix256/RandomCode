#include "LTHE.h"
#include <stdio.h>
#include <random>
#include <stdint.h>
#include <algorithm>

//=================================================================================
static size_t RandomSizeT (size_t min, size_t max) // inclusive
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_int<size_t> dist(min, max);
    return dist(mt);
}

//=================================================================================
static uint32_t RandomUint32 ()
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_int<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());
    return dist(mt);
}

namespace LTHE
{
    bool Encrypt (std::vector<float> values, size_t listSize, const char* fileName, std::vector<size_t>& keys)
    {
        // Make sure we have a list that is at least as long as the values we want to encrypt
        if (values.size() > listSize)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): values.size() > listSize.\n");
            return false;
        }

        // Generate a list of keys
        // Ideally you want to take the first M items of a cryptographically secure shuffle
        // of N items.
        // This could be done with format preserving encryption or some other method
        // to make it not roll and check, and also more secure random.
        keys.clear();
        for (size_t i = 0, c = values.size(); i < c; ++i)
        {
            size_t newKey;
            do
            {
                newKey = RandomSizeT(0, listSize - 1);
            } while (std::find(keys.begin(), keys.end(), newKey) != keys.end());
            keys.push_back(newKey);
        }

        // make a file of <listSize> random uint32's, which should be the same size as a float
        static_assert(sizeof(uint32_t) == sizeof(float), "This code assumes that a float and a uint32_t are the same size, but on your system it's not!");
        FILE *file = fopen(fileName, "w+b");
        if (!file)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not open %s for writing.\n", fileName);
            return false;
        }

        // Note: this may not be the most efficient way to generate this much random data or 
        // write it all to the file.
        // In a real crypto usage case, you'd want a crypto secure random number generator.
        // You'd also want to make sure the random numbers had the same properties as your
        // input values to help anonymize them better.
        // You could alternately just do all 2^32 floating point values which would definitely
        // anonymize the values you wanted to encrypt.
        for (size_t i = 0; i < listSize; ++i)
        {
            uint32_t value = RandomUint32();
            fwrite(&value, sizeof(value), 1, file);
        }

        // Now put the values in the file where they go, based on their key
        for (size_t i = 0, c = values.size(); i < c; ++i)
        {
            fseek(file, (long)(keys[i]*sizeof(uint32_t)), SEEK_SET);
            fwrite(&values[i], sizeof(values[i]), 1, file);
        }

        // close file and return success
        fclose(file);
        return true;
    }

    template <size_t NUMELEMENTS>
    static bool ReadBuffer (FILE* file, float (&buffer)[NUMELEMENTS], size_t& elementsRead)
    {
        
        return elementsRead > 0;
    }

    bool TransformHomomorphically (const char* fileName, TTransformationFN function)
    {
        // open the file if we can
        FILE *file = fopen(fileName, "r+b");
        if (!file)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not open %s for reading and writing.\n", fileName);
            return false;
        }

        // Process the data in the file and write it back out.
        // This could be done much better.
        // We could read more from the file at once.
        // We could use SIMD.
        // We could go multithreaded.
        // We could do this on the GPU for large data sets and longer transformations! Assuming data transfer time isn't too prohibitive.
        // We could decouple the disk access from processing, so it was reading while it was processing.
        const size_t c_bufferSize = 1024;
        std::vector<float> dataBuffer;
        dataBuffer.resize(c_bufferSize);
        size_t elementsRead;
        do
        {
            // read data from the buffer
            elementsRead = fread(&dataBuffer[0], sizeof(float), c_bufferSize, file);

            // process the data
            for (size_t i = 0; i < elementsRead; ++i)
                dataBuffer[i] = function(dataBuffer[i]);

            // write it back
            fseek(file, -(long)elementsRead * sizeof(float), SEEK_CUR);
            fwrite(&dataBuffer[0], sizeof(float), elementsRead, file);
        }
        while (elementsRead == c_bufferSize);

        // close file and return success
        fclose(file);
        return true;
    }

    bool Decrypt(const char* fileName, std::vector<float>& values, std::vector<size_t>& keys)
    {
        // Open the file if we can
        FILE *file = fopen(fileName, "rb");
        if (!file)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not open %s for reading.\n", fileName);
            return false;
        }

        // Read the values from the file.  The key is their location in the file.
        values.clear();
        for (size_t i = 0, c = keys.size(); i < c; ++i)
        {
            fseek(file, keys[i] * sizeof(float), SEEK_SET);
            float value;
            if (!fread(&value, sizeof(float), 1, file))
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not decrypt value for key.\n");
                fclose(file);
                return false;
            }
            values.push_back(value);
        }

        // Close file and return success
        fclose(file);
        return true;
    }
}