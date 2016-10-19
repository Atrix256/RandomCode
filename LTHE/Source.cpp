#include <stdio.h>
#include "LTHE.h"
#include <chrono>

//=================================================================================
// times a block of code
struct SBlockTimer
{
    SBlockTimer()
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~SBlockTimer()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        printf("    %0.2f seconds\n", seconds.count());
    }

    std::chrono::high_resolution_clock::time_point m_start;
};

//=================================================================================
float TransformDataUnitary (float& value)
{
    return (float)sqrt(value * 2.17f + 0.132);
}

//=================================================================================
float TransformDataBinary (float& value1, float value2)
{
    return (float)sqrt(value1 * value1 + value2 * value2);
}

//=================================================================================
struct SStruct
{
    uint8_t x, y, z;

    static SStruct Transform (const SStruct& b)
    {
        SStruct ret;
        ret.x = b.x * 2;
        ret.y = b.y * 3;
        ret.z = b.z * 4;
        return ret;
    }

    bool operator != (const SStruct& b) const
    {
        return b.x != x || b.y != y || b.z != z;
    }
};

//=================================================================================
int Test_FloatUnitaryOperation ()
{
    printf("\n----- " __FUNCTION__ " -----\n");

    // Encrypt the data
    printf("Encrypting data:  ");
    std::vector<float> secretValues = { 3.14159265359f, 435.0f };
    std::vector<size_t> keys;
    {
        SBlockTimer timer;
        if (!LTHE::Encrypt(secretValues, 10000000, "Encrypted.dat", keys))
        {
            fprintf(stderr, "Could not encrypt data.\n");
            return -1;
        }
    }

    // Transform the data
    printf("Transforming data:");
    {
        SBlockTimer timer;
        if (!LTHE::TransformHomomorphically<float>("Encrypted.dat", "Transformed.dat", TransformDataUnitary))
        {
            fprintf(stderr, "Could not transform encrypt data.\n");
            return -2;
        }
    }

    // Decrypt the data
    printf("Decrypting data:  ");
    std::vector<float> decryptedValues;
    {
        SBlockTimer timer;
        if (!LTHE::Decrypt("Transformed.dat", decryptedValues, keys))
        {
            fprintf(stderr, "Could not decrypt data.\n");
            return -3;
        }
    }

    // Verify the data
    printf("Verifying data:   ");
    {
        SBlockTimer timer;
        for (size_t i = 0, c = secretValues.size(); i < c; ++i)
        {
            if (TransformDataUnitary(secretValues[i]) != decryptedValues[i])
            {
                fprintf(stderr, "decrypted value mismatch!\n");
                return -4;
            }
        }
    }

    return 0;
}

//=================================================================================
int Test_FloatBinaryOperation ()
{
    printf("\n----- " __FUNCTION__ " -----\n");

    // Encrypt the data
    printf("Encrypting data:  ");
    std::vector<float> secretValues1 = { 3.14159265359f, 435.0f, 1.0f };
    std::vector<float> secretValues2 = { 1.0f, 5.0f, 9.0f };
    std::vector<size_t> keys;
    {
        SBlockTimer timer;
        if (!LTHE::Encrypt(secretValues1, 10000000, "Encrypted1.dat", keys))
        {
            fprintf(stderr, "Could not encrypt data.\n");
            return -1;
        }
        if (!LTHE::Encrypt(secretValues2, 10000000, "Encrypted2.dat", keys, false)) // reuse the keys made for secretValues1
        {
            fprintf(stderr, "Could not encrypt data.\n");
            return -1;
        }
    }

    // Transform the data
    printf("Transforming data:");
    {
        SBlockTimer timer;
        if (!LTHE::TransformHomomorphically<float>("Encrypted1.dat", "Encrypted2.dat", "Transformed.dat", TransformDataBinary))
        {
            fprintf(stderr, "Could not transform encrypt data.\n");
            return -2;
        }
    }

    // Decrypt the data
    printf("Decrypting data:  ");
    std::vector<float> decryptedValues;
    {
        SBlockTimer timer;
        if (!LTHE::Decrypt("Transformed.dat", decryptedValues, keys))
        {
            fprintf(stderr, "Could not decrypt data.\n");
            return -3;
        }
    }

    // Verify the data
    printf("Verifying data:   ");
    {
        SBlockTimer timer;
        for (size_t i = 0, c = secretValues1.size(); i < c; ++i)
        {
            if (TransformDataBinary(secretValues1[i], secretValues2[i]) != decryptedValues[i])
            {
                fprintf(stderr, "decrypted value mismatch!\n");
                return -4;
            }
        }
    }

    return 0;
}

//=================================================================================
int Test_StructUnitaryOperation ()
{
    printf("\n----- " __FUNCTION__ " -----\n");

    // Encrypt the data
    printf("Encrypting data:  ");
    std::vector<SStruct> secretValues = { {0,1,2},{ 3,4,5 },{ 6,7,8 } };
    std::vector<size_t> keys;
    {
        SBlockTimer timer;
        if (!LTHE::Encrypt(secretValues, 10000000, "Encrypted.dat", keys))
        {
            fprintf(stderr, "Could not encrypt data.\n");
            return -1;
        }
    }

    // Transform the data
    printf("Transforming data:");
    {
        SBlockTimer timer;
        if (!LTHE::TransformHomomorphically<SStruct>("Encrypted.dat", "Transformed.dat", SStruct::Transform))
        {
            fprintf(stderr, "Could not transform encrypt data.\n");
            return -2;
        }
    }

    // Decrypt the data
    printf("Decrypting data:  ");
    std::vector<SStruct> decryptedValues;
    {
        SBlockTimer timer;
        if (!LTHE::Decrypt("Transformed.dat", decryptedValues, keys))
        {
            fprintf(stderr, "Could not decrypt data.\n");
            return -3;
        }
    }

    // Verify the data
    printf("Verifying data:   ");
    {
        SBlockTimer timer;
        for (size_t i = 0, c = secretValues.size(); i < c; ++i)
        {
            if (SStruct::Transform(secretValues[i]) != decryptedValues[i])
            {
                fprintf(stderr, "decrypted value mismatch!\n");
                return -4;
            }
        }
    }

    return 0;
}

//=================================================================================
int main (int argc, char **argv)
{
    // test doing an operation on a single encrypted float
    int ret = Test_FloatUnitaryOperation();
    if (ret != 0)
    {
        system("pause");
        return ret;
    }

    // test doing an operation on two encrypted floats
    ret = Test_FloatBinaryOperation();
    if (ret != 0)
    {
        system("pause");
        return ret;
    }

    // test doing an operation on a single 3 byte struct
    ret = Test_StructUnitaryOperation();
    if (ret != 0)
    {
        system("pause");
        return ret;
    }
    
    printf("\nAll Tests Passed!\n\n");
    system("pause");
    return 0;
}

/*

Blog:
* Can encrypt M items by having the be in a list of N items
* Send that list of values to someone else
* They do operations on every item on the list and send it back

* further details
 * the N items should not make your data stand out. If your numbers are floats, don't generate a bunch of integers.
 * the larger the N, the more secure.
 * up to having the full space of values possible represented.
 * show how big that is in bytes for some value types.
 * when doing that, you don't even need to send the values to the person doing the computation, they can just send the results back.
  * could maybe even send back some of the data (like 2/3) and you could use the data if there, else call it a failed calculation and handle the failure gracefully, without asking again, to not give a hint if you got it or not.
 * The larger the M, the less secure
 * report some timing on your blog.
 * report file sizes - also report what they compress to, to show some amount of randomness?
 * When doing operations between multiple encrypted values, they need to be encrypted using the same keys! So thus, should also have the same number of encrypted and random items!

* For faster processing:
 * SIMD / multithreaded.
 * GPU
 ? can we benchmark?
 ! would be most robust to compare memory usage and execution speed to other FHE but i dont have time for that :p

--Links--
Fully Homomorphic SIMD Operations
http://homes.esat.kuleuven.be/~fvercaut/papers/DCC2011.pdf

*/