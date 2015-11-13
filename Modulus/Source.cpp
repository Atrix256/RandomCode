#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <stdint.h>
#include <string.h>
#include <memory>

typedef boost::multiprecision::cpp_int TINT;
typedef std::vector<TINT> TINTVec;

const float c_pi = 3.14159265359f;

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
void CalculateLookupTable (
    TINT &lut,
    const std::vector<uint64_t> &output,
    const TINTVec &keys,
    const TINT &keysLCM,
    const TINTVec &coefficients,
    size_t bitMask
)
{
    // figure out how much to multiply each coefficient by to make it have the specified modulus residue (remainder)
    lut = 0;
    for (size_t i = 0, c = keys.size(); i < c; ++i)
    {
        // we either want this term to be 0 or 1 mod the key.  if zero, we can multiply by zero, and
        // not add anything into the bit value!
        if ((output[i] & bitMask) == 0)
            continue;

        // if 1, use chinese remainder theorem
        TINT s, t;
        ExtendedEuclidianAlgorithm(coefficients[i], keys[i], s, t);
        lut = (lut + ((coefficients[i] * t) % keysLCM)) % keysLCM;
    }
}

//=================================================================================
template <typename TINPUT, typename TOUTPUT, typename LAMBDA>
void MakeModulus (TINTVec &luts, TINTVec &keys, LAMBDA &lambda)
{
    // to keep things simple, input sizes are being constrained.
    // Do this in x64 instead of win32 to make size_t 8 bytes instead of 4
    static_assert(sizeof(TINPUT) < sizeof(size_t), "Input too large");
    static_assert(sizeof(TOUTPUT) < sizeof(uint64_t), "Output too large");

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

    // iterate through each possible output bit, since each bit is it's own lut
    luts.resize(c_numOutputBits);
    for (size_t i = 0; i < c_numOutputBits; ++i)
    {
        const size_t bitMask = 1 << i;
        CalculateLookupTable(
            luts[i],
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
    // Look up tables encodes each bit, keys is used to decode each bit for specific
    // input values.
    TINTVec luts;
    TINTVec keys;

    // this is the function that it turns into modulus work
    typedef uint8_t TINPUT;
    typedef float TOUTPUT;
    auto lambda = [] (TINPUT input) -> TOUTPUT
    {
        return sin(((TOUTPUT)input) / 255.0f * 2.0f * c_pi);
    };

    MakeModulus<TINPUT, TOUTPUT>(luts, keys, lambda);

    // show last lut and key to show what kind of numbers they are
    std::cout << "Last Lut: " << *luts.rbegin() << "\n";
    std::cout << "Last Key: " << *keys.rbegin() << "\n";

    // Decode all input values
    std::cout << "\n" << sizeof(TINPUT) << " bytes input, " << sizeof(TOUTPUT) << " bytes output\n";
    for (size_t keyIndex = 0, keyCount = keys.size(); keyIndex < keyCount; ++keyIndex)
    {
        union
        {
            TOUTPUT value;
            size_t index;
        } result;

        result.index = 0;

        for (size_t lutIndex = 0, lutCount = luts.size(); lutIndex < lutCount; ++lutIndex)
        {
            TINT remainder = luts[lutIndex] % keys[keyIndex];
            size_t remainderSizeT = size_t(remainder);
            result.index += (remainderSizeT << lutIndex);
        }

        TINT remainder = luts[0] % keys[keyIndex];
        std::cout << "i:" << keyIndex << " o:" << result.value << "\n";
    }

    WaitForEnter();
    return 0;
}

/*

TODO:

? is union thing safe?

* talk about needing a large number of co-primes.  That this is what it's for (in FB post).

*/