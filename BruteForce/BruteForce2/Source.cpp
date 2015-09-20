#include <stdio.h>
#include <array>
#include <thread>
#include <vector>
#include <atomic>

#define MULTITHREADED() 1
#define MODULUSTWO() 1

typedef int TInteger;

const size_t c_numInputs = 2;
const size_t c_numKeys = 1 << c_numInputs;

const TInteger c_maxSearchValue = 50;

std::vector<TInteger> g_primeNumbers;

//=================================================================================
void WaitForEnter ()
{
    printf("\nPress Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
bool IncrementInputs (std::array<TInteger, c_numInputs>& inputs)
{
    for (size_t i = 0; i < c_numInputs; ++i)
    {
        if (inputs[i] < c_maxSearchValue)
        {
            ++inputs[i];
            return true;
        }

        inputs[i] = 1;
    }

    return false;
}

//=================================================================================
bool IncrementKeySpace (std::array<TInteger, c_numKeys>& keys)
{
    // we don't increment the highest key, since that defines our key space
    for (size_t i = 0; i < c_numKeys - 1; ++i)
    {
        if (keys[i] < c_maxSearchValue)
        {
            ++keys[i];
            return true;
        }

        keys[i] = 0;
    }

    return false;
}

//=================================================================================
bool InputSatisfiesConstraints (
    const std::array<TInteger, c_numInputs>& inputs,
    const std::array<TInteger, c_numKeys>& keys
)
{
    for (size_t input = 0; input < c_numInputs; ++input)
    {
        for (size_t key = 0; key < c_numKeys; ++key)
        {
            int output = (key & (1 << input)) != 0;
            #if MODULUSTWO()
            if (((inputs[input] % g_primeNumbers[keys[key]]) % 2) != output)
            #else
            if ((inputs[input] % g_primeNumbers[keys[key]]) != output)
            #endif
                return false;
        }
    }
    return true;
}

//=================================================================================
void PrintValues (
    const std::array<TInteger, c_numInputs>& inputs,
    const std::array<TInteger, c_numKeys>& keys
)
{
    printf("x=[");
    std::for_each(inputs.begin(), inputs.end(), [](const TInteger& input) {printf(" %i", input); });

    printf("]  k=[");
    std::for_each(keys.begin(), keys.end(), [](const TInteger& key) {printf(" %i", g_primeNumbers[key]); });
    printf("]\n");
}

//=================================================================================
void ProcessKeySet (const std::array<TInteger, c_numKeys>& keys)
{
    // get the maximum key
    TInteger maxKey = 0;
    std::for_each(
        keys.begin(),
        keys.end(),
        [&maxKey](const TInteger& key) {maxKey = std::max(maxKey, key); }
    );

    // cycle through our possible input values
    std::array<TInteger, c_numInputs> inputs;
    inputs.fill(0);

    static const std::array<TInteger, c_numInputs> inputs2 = { 15, 7 };
    static const std::array<TInteger, c_numKeys> keys2 = { 0, 4, 2, 1 };

    do 
    {
        if (InputSatisfiesConstraints(inputs, keys))
            PrintValues(inputs, keys);
    }
    while (IncrementInputs(inputs));
}

//=================================================================================
void ProcessKeySpace (TInteger keySpace)
{
    // initialize the starting keys
    std::array<TInteger, c_numKeys> keys;
    keys.fill(0);
    keys[c_numKeys - 1] = keySpace;

    // Process each set of keys in the key space
    do
    {
        ProcessKeySet(keys);
    }
    while (IncrementKeySpace(keys));
}

//=================================================================================
void PrintEquations ()
{
    for (size_t input = 0; input < c_numInputs; ++input)
    {
        for (size_t key = 0; key < c_numKeys; ++key)
        {
            int output = (key & (1 << input)) != 0;
            #if MODULUSTWO()
            printf("(x%i %% k%i) %% 2 = %i\n", input, key, output);
            #else
            printf("x%i %% k%i = %i\n", input, key, output);
            #endif
        }
        printf("\n");
    }
}

//=================================================================================
void ThreadFunction (std::atomic<TInteger>& nextKeySpace)
{
    TInteger keySpace;
    while ((keySpace = nextKeySpace++) <= c_maxSearchValue)
    {
        printf("%i / %i\n", keySpace, c_maxSearchValue);
        ProcessKeySpace(keySpace);
    }
}

//=================================================================================
void LoadPrimeNumbers ()
{
    FILE *file = fopen("primes-to-100k.txt","rt");
    if (file)
    {
        TInteger value;
        while (fscanf(file, "%i", &value) == 1)
            g_primeNumbers.push_back(value);
        fclose(file);
    }
}

//=================================================================================
int main (int argc, char **argv)
{
    // Load the prime numbers
    LoadPrimeNumbers();

    // show the equations we want to solve
    PrintEquations();

    // print the number of brute force values being checked
    float values = pow(float(c_maxSearchValue), float(c_numInputs + c_numKeys));
    printf("%0.2f values to permute\n", values);

    // report how many threads are used
    auto numThreads = std::thread::hardware_concurrency();
    #if !MULTITHREADED()
    numThreads = 1;
    #endif
    printf("%i threads\n\n", numThreads);

    // spin up the threads and then wait for it to be done
    std::atomic<TInteger> nextKeySpace = 0;
    std::vector<std::thread> threads;
    for (size_t i = 0, c = numThreads; i < c; ++i)
        threads.push_back(std::thread([&nextKeySpace]() { ThreadFunction(nextKeySpace); }));
    for_each(threads.begin(), threads.end(), [](std::thread& t) { t.join(); });

    WaitForEnter();
    return 0;
}

/*
TODO:
* time the results
* put solutions into a per thread list and report at end? it's stomping on eachother right now in the printf's
*/