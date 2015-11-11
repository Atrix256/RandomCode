#include <vector>
#include <boost/multiprecision/cpp_int.hpp>
#include <stdint.h>
#include <string.h>
#include <array>
#include <memory>

typedef boost::multiprecision::cpp_int TINT;
typedef std::vector<TINT> TINTVec;

/*
//=================================================================================
bool IncrementMemory (uint8_t *memory, size_t size)
{
    size_t i = 0;

    do
    {
        ++memory[i];
        if (memory[i] != 0)
            return true;
        ++i;
    }
    while (i < size);

    return false;
}
*/

//=================================================================================
template <typename TINPUT, typename TOUTPUT, typename LAMBDA>
void MakeModulus (TINTVec &programs, TINTVec &keys, LAMBDA &lambda)
{
    // to keep things simple, sizes are being constrained.
    static_assert(sizeof(TINPUT) < sizeof(size_t), "Input too large");
    static_assert(sizeof(TOUTPUT) < sizeof(size_t), "Output too large");

    // gather all the input to output mappings by permuting the input space
    // and storing the output for each input index
    const size_t numInputValues = 1 << ((size_t)(sizeof(TINPUT) * 8));
    typedef std::array<TOUTPUT, numInputValues> TOutputStorage;
    std::unique_ptr<TOutputStorage> output = std::make_unique<TOutputStorage>();
    union
    {
        TINPUT value;
        size_t index;
    } input;

    for (input.index = 0; input.index < numInputValues; ++input.index)
        (*output)[input.index] = lambda(input.value);

    // TODO: solve for each output bit, yadda yadda
    int ijkl = 0;
}

//=================================================================================
int main (int argc, char **argv)
{
    TINTVec programs;
    TINTVec keys;

    auto lambda = [](uint16_t input) -> int8_t
    {
        return input;
    };

    MakeModulus<uint16_t, int8_t>(programs, keys, lambda);

    return 0;
}

/*

TODO:
* make as general a thing as possible to modularize stuff.
* do something compelling, line sin maybe.
* print out all permutations and verify they are correct.

*/