#include <stdio.h>
#include <stdint.h>
#include <random>

typedef uint64_t uint64;

const size_t c_numKeyBits = 4;

#define Assert(x) if (!(x)) ((int*)nullptr)[0] = 0;

//=================================================================================
// Replace with something crypto secure if desired
uint8_t Random_uint8_t()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

int RandomInt(int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

bool RandomBool()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    return dis(gen) == 1;
}

//=================================================================================
void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
uint64 GenerateKey()
{
    // Generate an odd random number between 2^(N-1) and 2^N 
    // Where N is the number of bits in our key
    uint64 key = 0;
    for (size_t i = 0; i < c_numKeyBits; ++i)
    {
        bool value;
        if (i == 0 || i == c_numKeyBits - 1)
            value = true;
        else
            value = RandomBool();

        if (value)
            key |= uint64(1) << uint64(i);
    }
    return key;
}

//=================================================================================
uint64 Encrypt(uint64 key, bool value)
{
    // TODO: generate q and r randomly. based on what exactly?
    uint64 q = 3;
    uint64 r = RandomInt(0, 1 << (c_numKeyBits - 2));  // r must be smaller than p / 4

    uint64 m = value ? 1 : 0;

    uint64 c = key * q + 2 * r + m;
    return c;
}

//=================================================================================
uint64 XOR(uint64 A, uint64 B)
{
    return A + B;
}

//=================================================================================
uint64 AND(uint64 A, uint64 B)
{
    return A * B;
}

//=================================================================================
bool Decrypt(uint64 key, uint64 value)
{
    uint64 blah = value % key;
    return blah & 1 ? true : false;
}

//=================================================================================
int main(int argc, char **argv)
{
    for (int i = 0; i < 1000; ++i)
    {
        uint64 key = GenerateKey();
        uint64 falseBit = Encrypt(key, false);
        uint64 trueBit = Encrypt(key, true);

        bool falseBitDecrypted = Decrypt(key, falseBit);
        bool trueBitDecrypted = Decrypt(key, trueBit);

        Assert(falseBitDecrypted == false);
        Assert(trueBitDecrypted == true);

        uint64 falseXorTrue = XOR(falseBit, trueBit);
        Assert(Decrypt(key, falseXorTrue) == true);

        uint64 falseAndTrue = AND(falseBit, trueBit);
        Assert(Decrypt(key, falseAndTrue) == false);
    }

    WaitForEnter();
    return 0;
}

/*

TODO:
* paper: https://eprint.iacr.org/2009/616.pdf
* make it work
* understand it's insecurities
* generalize this stuff (AND and XOR)
* figure out bootstrapping?
* could generate the key better.  generate bytes at a time, mask away the parts we don't need, or on the front and back bits
* what happens when numbers roll over

* how should we randomly select Q and R?  there are some recomendations, why are they secure?

? is there a way to introduce error to this?

? why is the key a: random odd number between 2^(N-1) and 2^N

*/