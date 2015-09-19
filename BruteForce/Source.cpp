#include <stdio.h>
#include <array>
#include <thread>
#include <vector>
#include <atomic>

typedef int TInteger;

const size_t c_numInputs = 3;
const size_t c_numKeys = 1 << c_numInputs;

const TInteger c_maxSearchValue = 7;

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

        keys[i] = 1;
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
            if ((inputs[input] % keys[key]) % 2 != output)
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
    printf("I=[");
    std::for_each(inputs.begin(), inputs.end(), [](const TInteger& input) {printf(" %i", input); });

    printf("]  K=[");
    std::for_each(keys.begin(), keys.end(), [](const TInteger& key) {printf(" %i", key); });
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
    keys.fill(1);
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
            printf("(x%i %% k%i) %% 2 = %i\n", input, key, output);
        }
        printf("\n");
    }
}

//=================================================================================
void ThreadFunction (std::atomic<TInteger>& nextKeySpace)
{
    TInteger keySpace;
    while ((keySpace = ++nextKeySpace) <= c_maxSearchValue)
    {
        printf("%i / %i\n", keySpace, c_maxSearchValue);
        ProcessKeySpace(keySpace);
    }
}

//=================================================================================
int main (int argc, char **argv)
{
    // show the equations we want to solve
    PrintEquations();

    // report how many threads are used
    auto numThreads = std::thread::hardware_concurrency();
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
* put solutions into a per thread list and report at end? it's stomping on eachother right now.
? are there any where the keys are all even
 * probably not. I let it run up to 40, for max number = 100 and it didnt find any
* test this in homomorphic setting (n bit adder?)
* try and figure it out mathematically
* after proving that this stuff works homomorphically, gather up what needs to be done, and possible avenues to explore, and present to ben to see if he wants to collaborate.
* make a private github and share w/ him if he wants in

Paper Research TODO's
* find math to solve the constraints.  brute force is slow as f00k
 * look into successive substitution to solve these equations? https://en.wikipedia.org/wiki/Method_of_successive_substitution
* is there a way to have even keys? i don't think so but...
* how about the other way(s) to implement homomorphic encryption.  Does this apply there? are any more interesting / easily done or have better properties?
? will modulus switching work?
? maybe bootstrapping would be better


FIRST ANSWER:
3,7,1,5,3,2

x0 = 3
x1 = 7
k0 = 1
k1 = 5
k2 = 3
k3 = 2

((3 % 1) % 2) == 0
((3 % 5) % 2) == 1
((3 % 3) % 2) == 0
((3 % 2) % 2) == 1
((7 % 1) % 2) == 0
((7 % 5) % 2) == 0
((7 % 3) % 2) == 1
((7 % 2) % 2) == 1

OTHER ANSWER:
((x0 % k0) % 2) == 0
((x0 % k1) % 2) == 1
((x0 % k2) % 2) == 0
((x0 % k3) % 2) == 1
((x1 % k0) % 2) == 0
((x1 % k1) % 2) == 0
((x1 % k2) % 2) == 1
((x1 % k3) % 2) == 1

5, 98, 3, 99, 5, 97

x0 = 5
x1 = 98
k0 = 3
k1 = 99
k2 = 5
k3 = 97

((5 % 3) % 2) == 0
((5 % 99) % 2) == 1
((5 % 5) % 2) == 0
((5 % 97) % 2) == 1
((98 % 3) % 2) == 0
((98 % 99) % 2) == 0
((98 % 5) % 2) == 1
((98 % 97) % 2) == 1
*/