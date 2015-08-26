#include <stdio.h>
#include <stdint.h>
#include <random>

typedef uint64_t uint64;

const size_t c_numKeyBits = 4;
const size_t c_numResidueBits = c_numKeyBits*c_numKeyBits;
const size_t c_numQBits = c_numKeyBits*c_numKeyBits*c_numKeyBits*c_numKeyBits*c_numKeyBits;

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
    // Generate an odd random number in [2^(N-1), 2^N)
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
    uint64 q = 1;

    // r is the "noise"
    uint64 r = 0;// RandomInt(0, key / 4 - 1);  // r must be smaller than p / 4

    uint64 m = value ? 1 : 0;

    uint64 c = key * q + 2 * r + m;
    return c;
}

//=================================================================================
bool Decrypt(uint64 key, uint64 value)
{
    uint64 blah = value % key;
    return blah & 1 ? true : false;
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

    int a = 3;
    int b = 4;
    int c = 5;
    int d = 6;

    int e = 4 % 3;
    int f = 5 % 3;
    int g = 5 % 4;
    int h = 6 % 4;

    WaitForEnter();
    return 0;
}

/*

TODO:
* this looks promising: https://github.com/coron/fhe
 * compressed public key and modulus switching!
* paper: https://eprint.iacr.org/2009/616.pdf
* make it work
* understand it's insecurities
 * what if you don't add error (remove the residue r term)? how does that change security
* generalize this stuff (AND and XOR)
* figure out bootstrapping?
* could generate the key better.  generate bytes at a time, mask away the parts we don't need, or on the front and back bits
* what happens when numbers roll over?
 * can we mask it (let it roll over), or do we need to do multi precision math stuff?
* do we need to do it first without bootstrapping? then an updated blog post wqith bootstrapping

? what is the appropriate number of bits for each thing?

? why does this work?
* how should we randomly select Q and R?  there are some recomendations, why are they secure?

? why is the key a: random odd number between 2^(N-1) and 2^N
 * why odd number:  http://crypto.stackexchange.com/questions/18454/public-key-in-fully-homomorphic-encryption-over-the-integers
  * i think that answers it, but think about it / reason it out.
 ? why 2^(N-1) to 2^N. aka, why always have high bit set?

* do symetric key, then public / private after

! make a "tiny somewhat homomorphic" implementation using just uint32s and one operation first! then more complex stuff.

Blog:
! WOW.  Addition litteraly adds the error, and multiplication multiplies it.  show this. by showing number % key for false and true bits and after anding and xoring.
! AND and OR didn't work when noise got too large and caused the wrong answers to come out

*/