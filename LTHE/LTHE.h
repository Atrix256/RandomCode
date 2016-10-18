#pragma once
#include <vector>
#include <functional>
#include <random>

// A static class with template functions in it.
// A namespace would be nice, except I want to hide some things as private.
class LTHE
{
public:

    //=================================================================================
    template <typename T>
    static bool Encrypt (std::vector<T> values, size_t listSize, const char* fileName, std::vector<size_t>& keys)
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
                newKey = RandomInt<size_t>(0, listSize - 1);
            }
            while (std::find(keys.begin(), keys.end(), newKey) != keys.end());
            keys.push_back(newKey);
        }

        // make a file of random values, size of T, count of <listSize> 
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
        // Like if your numbers are not whole numbers, you don't want to generate only whole numbers.
        // Or if your numbers are salaries, you may not want purely random values, but more "salaryish"
        // looking numbers.
        // You could alternately just do all 2^N possible values which would definitely anonymize
        // the values you wanted to encrypt.  This is maximum security, but also takes most
        // memory and most processing time.
        size_t numUint64s = (listSize * sizeof(T)) / sizeof(uint64_t);
        size_t numExtraBytes = (listSize * sizeof(T)) % sizeof(uint64_t);
        for (size_t i = 0; i < numUint64s; ++i)
        {
            uint64_t value = RandomInt<uint64_t>();
            fwrite(&value, sizeof(value), 1, file);
        }
        for (size_t i = 0; i < numExtraBytes; ++i)
        {
            uint8_t value = RandomInt<uint8_t>();
            fwrite(&value, sizeof(value), 1, file);
        }

        // Now put the values in the file where they go, based on their key
        for (size_t i = 0, c = values.size(); i < c; ++i)
        {
            fseek(file, (long)(keys[i]*sizeof(T)), SEEK_SET);
            fwrite(&values[i], sizeof(values[i]), 1, file);
        }

        // close file and return success
        fclose(file);
        return true;
    }

    //=================================================================================
    template <typename T>
    static bool TransformHomomorphically (const char* fileName, const std::function<T(T)>& function)
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
        std::vector<T> dataBuffer;
        dataBuffer.resize(c_bufferSize);
        size_t elementsRead;
        do
        {
            // read data from the buffer
            elementsRead = fread(&dataBuffer[0], sizeof(T), c_bufferSize, file);

            // process the data
            for (size_t i = 0; i < elementsRead; ++i)
                dataBuffer[i] = function(dataBuffer[i]);

            // write it back
            fseek(file, -(long)elementsRead * sizeof(T), SEEK_CUR);
            fwrite(&dataBuffer[0], sizeof(T), elementsRead, file);
        }
        while (elementsRead == c_bufferSize);

        // close file and return success
        fclose(file);
        return true;
    }

    //=================================================================================
    template <typename T>
    static bool Decrypt (const char* fileName, std::vector<T>& values, std::vector<size_t>& keys)
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
            fseek(file, (long)(keys[i] * sizeof(T)), SEEK_SET);
            T value;
            if (!fread(&value, sizeof(T), 1, file))
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

private:
    template <typename T>
    static T RandomInt (T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
    {
        static std::random_device rd;
        static std::mt19937 mt(rd());
        static std::uniform_int<T> dist(min, max);
        return dist(mt);
    }
};