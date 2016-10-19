#pragma once
#include <vector>
#include <random>

// A static class with template functions in it.
// A namespace would be nice, except I want to hide some things as private.
class LTHE
{
public:

    //=================================================================================
    template <typename T>
    static bool Encrypt (std::vector<T> values, size_t listSize, const char* fileName, std::vector<size_t>& keys, bool generateKeys = true)
    {
        // Make sure we have a list that is at least as long as the values we want to encrypt
        if (values.size() > listSize)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): values.size() > listSize.\n");
            return false;
        }

        // Generate a list of keys if we are told to
        // Ideally you want to take the first M items of a cryptographically secure shuffle
        // of N items.
        // This could be done with format preserving encryption or some other method
        // to make it not roll and check, and also more secure random.
        if (generateKeys)
        {
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
        size_t numUint32s = (listSize * sizeof(T)) / sizeof(uint32_t);
        size_t numExtraBytes = (listSize * sizeof(T)) % sizeof(uint32_t);
        for (size_t i = 0; i < numUint32s; ++i)
        {
            uint32_t value = RandomInt<uint32_t>();
            if (fwrite(&value, sizeof(value), 1, file) != 1)
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not write random numbers (uint32s).\n");
                fclose(file);
                return false;
            }
        }
        for (size_t i = 0; i < numExtraBytes; ++i)
        {
            uint8_t value = RandomInt<uint8_t>();
            if (fwrite(&value, sizeof(value), 1, file) != 1)
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not write random numbers (extra bytes).\n");
                fclose(file);
                return false;
            }
        }

        // Now put the values in the file where they go, based on their key
        for (size_t i = 0, c = values.size(); i < c; ++i)
        {
            long pos = (long)(keys[i] * sizeof(T));
            if (fseek(file, pos, SEEK_SET) != 0)
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not fseek.\n");
                fclose(file);
                return false;
            }
            if (fwrite(&values[i], sizeof(values[i]), 1, file) != 1)
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not write secret value.\n");
                fclose(file);
                return false;
            }
        }

        // close file and return success
        fclose(file);
        return true;
    }

    //=================================================================================
    template <typename T, typename LAMBDA>
    static bool TransformHomomorphically (const char* srcFileName, const char* destFileName, const LAMBDA& function)
    {
        // open the source and dest file if we can
        FILE *srcFile = fopen(srcFileName, "rb");
        if (!srcFile)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not open %s for reading.\n", srcFileName);
            return false;
        }
        FILE *destFile = fopen(destFileName, "w+b");
        if (!destFile)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not open %s for writing.\n", destFileName);
            fclose(srcFile);
            return false;
        }

        // Process the data in the file and write it back out.
        // This could be done much better.
        // We could read more from the file at once.
        // We could use SIMD.
        // We could go multithreaded.
        // We could do this on the GPU for large data sets and longer transformations! Assuming data transfer time isn't too prohibitive.
        // We could decouple the disk access from processing, so it was reading and writing while it was processing.
        const size_t c_bufferSize = 1024;
        std::vector<T> dataBuffer;
        dataBuffer.resize(c_bufferSize);
        size_t elementsRead;
        do
        {
            // read data from the source file
            elementsRead = fread(&dataBuffer[0], sizeof(T), c_bufferSize, srcFile);

            // transform the data
            for (size_t i = 0; i < elementsRead; ++i)
                dataBuffer[i] = function(dataBuffer[i]);

            // write the transformed data to the dest file
            if (fwrite(&dataBuffer[0], sizeof(T), elementsRead, destFile) != elementsRead)
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not write transformed elements.\n");
                fclose(srcFile);
                fclose(destFile);
                return false;
            }
        }
        while (!feof(srcFile));

        // close files and return success
        fclose(srcFile);
        fclose(destFile);
        return true;
    }

    //=================================================================================
    template <typename T, typename LAMBDA>
    static bool TransformHomomorphically (const char* src1FileName, const char* src2FileName, const char* destFileName, const LAMBDA& function)
    {
        // open the source and dest file if we can
        FILE *srcFile1 = fopen(src1FileName, "rb");
        if (!srcFile1)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not open %s for reading.\n", src1FileName);
            return false;
        }
        FILE *srcFile2 = fopen(src2FileName, "rb");
        if (!srcFile2)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not open %s for reading.\n", src2FileName);
            fclose(srcFile1);
            return false;
        }
        FILE *destFile = fopen(destFileName, "w+b");
        if (!destFile)
        {
            fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not open %s for writing.\n", destFileName);
            fclose(srcFile1);
            fclose(srcFile2);
            return false;
        }

        // Process the data in the file and write it back out.
        // This could be done much better.
        // We could read more from the file at once.
        // We could use SIMD.
        // We could go multithreaded.
        // We could do this on the GPU for large data sets and longer transformations! Assuming data transfer time isn't too prohibitive.
        // We could decouple the disk access from processing, so it was reading and writing while it was processing.
        const size_t c_bufferSize = 1024;
        std::vector<T> dataBuffer1, dataBuffer2;
        dataBuffer1.resize(c_bufferSize);
        dataBuffer2.resize(c_bufferSize);
        size_t elementsRead1;
        size_t elementsRead2;
        do
        {
            // read data from the source files
            elementsRead1 = fread(&dataBuffer1[0], sizeof(T), c_bufferSize, srcFile1);
            elementsRead2 = fread(&dataBuffer2[0], sizeof(T), c_bufferSize, srcFile2);

            if (elementsRead1 != elementsRead2)
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Different numbers of elements in each file!\n");
                fclose(srcFile1);
                fclose(srcFile2);
                fclose(destFile);
                return false;
            }

            // transform the data
            for (size_t i = 0; i < elementsRead1; ++i)
                dataBuffer1[i] = function(dataBuffer1[i], dataBuffer2[i]);

            // write the transformed data to the dest file
            if (fwrite(&dataBuffer1[0], sizeof(T), elementsRead1, destFile) != elementsRead1)
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not write transformed elements.\n");
                fclose(srcFile1);
                fclose(srcFile2);
                fclose(destFile);
                return false;
            }
        }
        while (!feof(srcFile1));

        // close files and return success
        fclose(srcFile1);
        fclose(srcFile2);
        fclose(destFile);
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
            long pos = (long)(keys[i] * sizeof(T));
            if (fseek(file, pos, SEEK_SET) != 0)
            {
                fprintf(stderr, "ERROR in " __FUNCTION__ "(): Could not fseek.\n");
                fclose(file);
                return false;
            }
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