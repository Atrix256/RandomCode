#include <stdio.h>
#include "LTHE.h"

//=================================================================================
/*
// times a block of code
struct SBlockTimer
{
    SBlockTimer(SBlockTimerAggregator& aggregator)
        : m_aggregator(aggregator)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~SBlockTimer()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        float milliseconds = seconds.count() * 1000.0f;
        m_aggregator.AddSample(milliseconds);

        if (c_verboseSamples)
            printf("%s %i/%u: %0.2f ms  (avg = %0.2f ms. std dev = %0.2f ms) \n", m_aggregator.m_label, m_aggregator.m_numSamples, c_testSamples, milliseconds, m_aggregator.GetAverage(), m_aggregator.GetStandardDeviation());
    }

    SBlockTimerAggregator&                m_aggregator;
    std::chrono::high_resolution_clock::time_point m_start;
};
*/

//=================================================================================
float TransformData (float value)
{
    return (float)sqrt(value * 2.17f + 0.132);
}

//=================================================================================
int main (int argc, char **argv)
{
    // Encrypt the data
    printf("Encrypting data...\n");
    std::vector<float> secretValues = { 3.14159265359f, 435.0f };
    std::vector<size_t> keys;
    if (!LTHE::Encrypt(secretValues, 100000000, "Encrypted.dat", keys))
    {
        fprintf(stderr, "Could not encrypt data.\n");
        return -1;
    }

    // Transform the data
    printf("Transforming data...\n");
    if (!LTHE::TransformHomomorphically("Encrypted.dat", TransformData))
    {
        fprintf(stderr, "Could not transform encrypt data.\n");
        return -2;
    }

    // Decrypt the data
    printf("Decrypting data...\n");
    std::vector<float> decryptedValues;
    if (!LTHE::Decrypt("Encrypted.dat", decryptedValues, keys))
    {
        fprintf(stderr, "Could not decrypt data.\n");
        return -3;
    }

    // Verify the data
    printf("Verifying data...\n");
    for (size_t i = 0, c = secretValues.size(); i < c; ++i)
    {
        if (TransformData(secretValues[i]) != decryptedValues[i])
        {
            fprintf(stderr, "decrypted value mismatch!\n");
            return -4;
        }
    }

    printf("Finished, everything checked out!\n");
    return 0;
}

/*

TODO:
* instrument with timing!
* make it template based, not just floats! it'll be a header only library then.

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

* For faster processing:
 * SIMD / multithreaded.
 * GPU
 ? can we benchmark?
 ! would be most robust to compare memory usage and execution speed to other FHE but i dont have time for that :p

--Links--
Fully Homomorphic SIMD Operations
http://homes.esat.kuleuven.be/~fvercaut/papers/DCC2011.pdf

*/