#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <stdint.h>
#include <string.h>
#include <memory>

typedef boost::multiprecision::cpp_int TINT;
typedef std::vector<TINT> TINTVec;

//=================================================================================
void WaitForEnter ()
{
    printf("\nPress Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
static TINT ExtendedEuclidianAlgorithm (TINT smaller, TINT larger, TINT &s, TINT &t)
{
    // make sure A <= B before starting
    bool swapped = false;
    if (larger < smaller)
    {
        swapped = true;
        std::swap(smaller, larger);
    }

    // set up our storage for the loop.  We only need the last two values so will
    // just use a 2 entry circular buffer for each data item
    std::array<TINT, 2> remainders = { larger, smaller };
    std::array<TINT, 2> ss = { 1, 0 };
    std::array<TINT, 2> ts = { 0, 1 };
    size_t indexNeg2 = 0;
    size_t indexNeg1 = 1;

    // loop
    while (1)
    {
        // calculate our new quotient and remainder
        TINT newQuotient = remainders[indexNeg2] / remainders[indexNeg1];
        TINT newRemainder = remainders[indexNeg2] - newQuotient * remainders[indexNeg1];

        // if our remainder is zero we are done.
        if (newRemainder == 0)
        {
            // return our s and t values as well as the quotient as the GCD
            s = ss[indexNeg1];
            t = ts[indexNeg1];
            if (swapped)
                std::swap(s, t);

            // if t < 0, add the modulus divisor to it, to make it positive
            if (t < 0)
                t += smaller;
            return remainders[indexNeg1];
        }

        // calculate this round's s and t
        TINT newS = ss[indexNeg2] - newQuotient * ss[indexNeg1];
        TINT newT = ts[indexNeg2] - newQuotient * ts[indexNeg1];

        // store our values for the next iteration
        remainders[indexNeg2] = newRemainder;
        ss[indexNeg2] = newS;
        ts[indexNeg2] = newT;

        // move to the next iteration
        std::swap(indexNeg1, indexNeg2);
    }
}

//=================================================================================
void MakeKey (TINTVec &keys, TINT &keysLCM, size_t index)
{
    // if this is the first key, use 3
    if (index == 0)
    {
        keys[index] = 3;
        keysLCM = keys[index];
        return;
    }

    // Else start at the last number and keep checking odd numbers beyond that
    // until you find one that is co-prime.
    TINT nextNumber = keys[index - 1];
    while (1)
    {
        nextNumber += 2;
        if (std::all_of(
            keys.begin(),
            keys.begin() + index,
            [&nextNumber] (const TINT& v) -> bool
            {
                TINT s, t;
                return ExtendedEuclidianAlgorithm(v, nextNumber, s, t) == 1;
            }))
        {
            keys[index] = nextNumber;
            keysLCM *= nextNumber;
            return;
        }
    }
}

//=================================================================================
void CalculateProgram (
    TINT &program,
    const std::vector<uint64_t> &output,
    const TINTVec &keys,
    const TINT &keysLCM,
    const TINTVec &coefficients,
    size_t bitMask
)
{
    // figure out how much to multiply each coefficient by to make it have the specified modulus residue (remainder)
    program = 0;
    for (size_t i = 0, c = keys.size(); i < c; ++i)
    {
        // we either want this term to be 0 or 1 mod the key.  if zero, we can multiply by zero, and
        // not add anything into the bit value!
        if ((output[i] & bitMask) == 0)
            continue;

        // if 1, use chinese remainder theorem
        TINT s, t;
        ExtendedEuclidianAlgorithm(coefficients[i], keys[i], s, t);
        program = (program + ((coefficients[i] * t) % keysLCM)) % keysLCM;
    }
}

//=================================================================================
template <typename TINPUT, typename TOUTPUT, typename LAMBDA>
void MakeModulus (TINTVec &programs, TINTVec &keys, LAMBDA &lambda)
{
    // to keep things simple, input sizes are being constrained.
    // Do this in x64 instead of win32 to make size_t 8 bytes instead of 4
    static_assert(sizeof(TINPUT) < sizeof(size_t), "Input too large");
    static_assert(sizeof(TOUTPUT) < sizeof(uint64_t), "Input too large");

    // calculate some constants
    const size_t c_numInputBits = sizeof(TINPUT) * 8;
    const size_t c_numInputValues = 1 << c_numInputBits;
    const size_t c_numOutputBits = sizeof(TOUTPUT) * 8;

    // Generate the keys (coprimes)
    TINT keysLCM;
    keys.resize(c_numInputValues);
    for (size_t index = 0; index < c_numInputValues; ++index)
        MakeKey(keys, keysLCM, index);

    // calculate co-efficients for use in the chinese remainder theorem
    TINTVec coefficients;
    coefficients.resize(c_numInputValues);
    fill(coefficients.begin(), coefficients.end(), 1);
    for (size_t i = 0; i < c_numInputValues; ++i)
    {
        for (size_t j = 0; j < c_numInputValues; ++j)
        {
            if (i != j)
                coefficients[i] *= keys[j];
        }
    }

    // gather all the input to output mappings by permuting the input space
    // and storing the output for each input index
    std::vector<uint64_t> output;
    output.resize(c_numInputValues);
    union
    {
        TINPUT value;
        size_t index;
    } input;
    union
    {
        TOUTPUT value;
        size_t index;
    } outputConverter;

    for (input.index = 0; input.index < c_numInputValues; ++input.index)
    {
        outputConverter.value = lambda(input.value);
        output[input.index] = outputConverter.index;
    }

    // iterate through each possible output bit, since each bit is it's own program
    programs.resize(c_numOutputBits);
    for (size_t i = 0; i < c_numOutputBits; ++i)
    {
        const size_t bitMask = 1 << i;
        CalculateProgram(
            programs[i],
            output,
            keys,
            keysLCM,
            coefficients,
            bitMask
        );
    }
}

//=================================================================================
int main (int argc, char **argv)
{
    TINTVec programs;
    TINTVec keys;

    // this is the function that it turns into modulus work
    typedef uint8_t TINPUT;
    typedef float TOUTPUT;
    auto lambda = [](TINPUT input) -> TOUTPUT
    {
        return ((TOUTPUT)input)/255.0f;
    };

    MakeModulus<TINPUT, TOUTPUT>(programs, keys, lambda);

    // TODO: test that program is correct, print out values, etc!
    std::cout << sizeof(TINPUT) << " bytes input, " << sizeof(TOUTPUT) << " bytes output\n";
    for (size_t keyIndex = 0, keyCount = keys.size(); keyIndex < keyCount; ++keyIndex)
    {
        union
        {
            TOUTPUT value;
            size_t index;
        } result;

        result.index = 0;

        for (size_t programIndex = 0, programCount = programs.size(); programIndex < programCount; ++programIndex)
        {
            TINT remainder = programs[programIndex] % keys[keyIndex];
            size_t remainderSizeT = size_t(remainder);
            result.index += (remainderSizeT << programIndex);
        }

        TINT remainder = programs[0] % keys[keyIndex];
        std::cout << "i:" << keyIndex << " o:" << result.value << "\n";
    }

    WaitForEnter();
    return 0;
}

/*

TODO:
* make as general a thing as possible to modularize stuff.
* do something compelling, line sin maybe.
* print out all permutations and verify they are correct.
* can we auto-detect more types, so we don't need to keep repeating input and output types? could use a typedef perhaps but that kinda sucks.
* Note in post that increasing output bits isn't very expensive.
* test with strings? maybe do multiple tests
* time how long it takes to do each thing?

? is union thing safe?

*/