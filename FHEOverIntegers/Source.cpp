#include <stdio.h>
#include <stdint.h>
#include <random>
#include <array>
#include <inttypes.h>

typedef uint64_t uint64;

// Increase this value to increase the size of the key, and also the maximum
// size of the error allowed.
// If you set it too high, operations will fail when they run out of storage space
// in the 64 bit ints.  If you set it too low, you will not be able to do very many
// operations in a row.
// The recomended values for good security for these numbers are way too large to
// fit in a uint64, so adjusting them down to show their effects while using uint64s
const size_t c_numKeyBits = 15;
const size_t c_numNoiseBits = 3; //size_t(sqrt(c_numKeyBits));
const size_t c_numMultiplierBits = 4; //c_numKeyBits * c_numKeyBits * c_numKeyBits;

#define Assert(x) if (!(x)) ((int*)nullptr)[0] = 0;

//=================================================================================
// TODO: Replace with something crypto secure if desired!
uint64 RandomUint64 (uint64 min, uint64 max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<uint64> dis(min, max);
    return dis(gen);
}

//=================================================================================
void WaitForEnter ()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
uint64 GenerateKey ()
{
    // Generate an odd random number in [2^(N-1), 2^N)
    // N is the number of bits in our key
    // The key also defines the maximum amount of error allowed, and thus the number
    // of operations allowed in a row.
    uint64 key = RandomUint64(0, (uint64(1) << uint64(c_numKeyBits)) - 1);
    key = key | (uint64(1) << uint64(c_numKeyBits - 1));
    key = key | 1;
    return key;
}

//=================================================================================
bool Decrypt (uint64 key, uint64 value)
{
    return ((value % key) % 2) == 1;
}

//=================================================================================
uint64 Encrypt (uint64 key, bool value)
{
    uint64 keyMultiplier = RandomUint64(0, (1 << c_numMultiplierBits) - 2) + 1;
    uint64 noise = RandomUint64(0, (1 << c_numNoiseBits) - 1);
    uint64 ret = key * keyMultiplier + 2 * noise + (value ? 1 : 0);
    Assert(Decrypt(key, ret) == value);
    return ret;
}

//=================================================================================
uint64 XOR (uint64 A, uint64 B)
{
    return A + B;
}

//=================================================================================
uint64 AND (uint64 A, uint64 B)
{
    return A * B;
}

//=================================================================================
float GetErrorPercent (uint64 key, uint64 value)
{
    // Returns what % of maximum error this value has in it.  When error >= 100%
    // then we have hit our limit and start getting wrong answers.
    return 100.0f * float(value % key) / float(key);
}

//=================================================================================
uint64 FullAdder (uint64 A, uint64 B, uint64 &carryBit)
{
    // homomorphically add the encrypted bits A and B
    // return the single bit sum, and put the carry bit into carryBit
    // From http://en.wikipedia.org/w/index.php?title=Adder_(electronics)&oldid=381607326#Full_adder
    uint64 sumBit = XOR(XOR(A, B), carryBit);
    carryBit = XOR(AND(A, B), AND(carryBit, XOR(A, B)));
    return sumBit;
}

//=================================================================================
int main (int argc, char **argv)
{
    // run this test a bunch to show that it works.  If you get a divide by zero
    // in an Assert, that means that it failed, and hopefully it's because you
    // increased c_numKeyBits to be too large!
    printf("Verifying 10000 truth tables.  Details of first one:\n");
    for (int index = 0; index < 10000; ++index)
    {
        // make our key and a true and false bit
        uint64 key = GenerateKey();
        uint64 falseBit1 = Encrypt(key, false);
        uint64 falseBit2 = Encrypt(key, false);
        uint64 trueBit1  = Encrypt(key, true);
        uint64 trueBit2  = Encrypt(key, true);

        // report the results for the first iteration of the loop
        if (index == 0)
        {
            printf("Key 0x%" PRIx64 ", false = 0x%" PRIx64 ", 0x%" PRIx64 " true = 0x%" PRIx64 " 0x%" PRIx64 "\n", key, falseBit1, falseBit2, trueBit1, trueBit2);
            printf("  [0 xor 0] = 0   0x%" PRIx64 " xor(+) 0x%" PRIx64 " = 0x%" PRIx64 " (%i err=%0.2f%%)\n", falseBit1, falseBit2, XOR(falseBit1, falseBit2), Decrypt(key, XOR(falseBit1, falseBit2)) ? 1 : 0, GetErrorPercent(key, XOR(falseBit1, falseBit2)));
            printf("  [0 xor 1] = 1   0x%" PRIx64 " xor(+) 0x%" PRIx64 " = 0x%" PRIx64 " (%i err=%0.2f%%)\n", falseBit1, trueBit2 , XOR(falseBit1, trueBit2 ), Decrypt(key, XOR(falseBit1, trueBit2 )) ? 1 : 0, GetErrorPercent(key, XOR(falseBit1, trueBit2 )));
            printf("  [1 xor 0] = 1   0x%" PRIx64 " xor(+) 0x%" PRIx64 " = 0x%" PRIx64 " (%i err=%0.2f%%)\n", trueBit1 , falseBit2, XOR(trueBit1 , falseBit2), Decrypt(key, XOR(trueBit1 , falseBit2)) ? 1 : 0, GetErrorPercent(key, XOR(trueBit1 , falseBit2)));
            printf("  [1 xor 1] = 0   0x%" PRIx64 " xor(+) 0x%" PRIx64 " = 0x%" PRIx64 " (%i err=%0.2f%%)\n", trueBit1 , trueBit2 , XOR(trueBit1 , trueBit2 ), Decrypt(key, XOR(trueBit1 , trueBit2 )) ? 1 : 0, GetErrorPercent(key, XOR(trueBit1 , trueBit2 )));
            printf("  [0 and 0] = 0   0x%" PRIx64 " and(*) 0x%" PRIx64 " = 0x%" PRIx64 " (%i err=%0.2f%%)\n", falseBit1, falseBit2, AND(falseBit1, falseBit2), Decrypt(key, AND(falseBit1, falseBit2)) ? 1 : 0, GetErrorPercent(key, XOR(falseBit1, falseBit2)));
            printf("  [0 and 1] = 0   0x%" PRIx64 " and(*) 0x%" PRIx64 " = 0x%" PRIx64 " (%i err=%0.2f%%)\n", falseBit1, trueBit2 , AND(falseBit1, trueBit2 ), Decrypt(key, AND(falseBit1, trueBit2 )) ? 1 : 0, GetErrorPercent(key, XOR(falseBit1, trueBit2 )));
            printf("  [1 and 0] = 0   0x%" PRIx64 " and(*) 0x%" PRIx64 " = 0x%" PRIx64 " (%i err=%0.2f%%)\n", trueBit1 , falseBit2, AND(trueBit1 , falseBit2), Decrypt(key, AND(trueBit1 , falseBit2)) ? 1 : 0, GetErrorPercent(key, XOR(trueBit1 , falseBit2)));
            printf("  [1 and 1] = 1   0x%" PRIx64 " and(*) 0x%" PRIx64 " = 0x%" PRIx64 " (%i err=%0.2f%%)\n", trueBit1 , trueBit2 , AND(trueBit1 , trueBit2 ), Decrypt(key, AND(trueBit1 , trueBit2 )) ? 1 : 0, GetErrorPercent(key, XOR(trueBit1 , trueBit2 )));
        }

        // Verify truth tables for XOR and AND
        Assert(Decrypt(key, XOR(falseBit1, falseBit2)) == false);
        Assert(Decrypt(key, XOR(falseBit1, trueBit2 )) == true );
        Assert(Decrypt(key, XOR(trueBit1 , falseBit2)) == true );
        Assert(Decrypt(key, XOR(trueBit1 , trueBit2 )) == false);

        Assert(Decrypt(key, AND(falseBit1, falseBit2)) == false);
        Assert(Decrypt(key, AND(falseBit1, trueBit2 )) == false);
        Assert(Decrypt(key, AND(trueBit1 , falseBit2)) == false);
        Assert(Decrypt(key, AND(trueBit1 , trueBit2 )) == true );
    }

    // Do multi bit addition as an example of using compound circuits to
    // do meaningful work.
    const size_t c_numBitsAdded = 3;
    printf("\nDoing 10000 Multibit Additions.  Details of first one:\n");
    std::array<uint64, c_numBitsAdded> numberAEncrypted;
    std::array<uint64, c_numBitsAdded> numberBEncrypted;
    std::array<uint64, c_numBitsAdded> resultEncrypted;
    std::array<uint64, c_numBitsAdded> carryEncrypted;
    for (int index = 0; index < 10000; ++index)
    {
        // generate the numbers we want to add
        uint64 numberA = RandomUint64(0, (1 << c_numBitsAdded) - 1);
        uint64 numberB = RandomUint64(0, (1 << c_numBitsAdded) - 1);

        // generate our key
        uint64 key = GenerateKey();

        // encrypt our bits
        for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
        {
            numberAEncrypted[bitIndex] = Encrypt(key, (numberA & (uint64(1) << uint64(bitIndex))) != 0);
            numberBEncrypted[bitIndex] = Encrypt(key, (numberB & (uint64(1) << uint64(bitIndex))) != 0);
        }

        // do our multi bit addition!
        // we could initialize the carry bit to 0 or the encrypted value of 0. either one works since 0 and 1
        // are also poor encryptions of 0 and 1 in this scheme!
        uint64 carryBit = Encrypt(key, false);
        for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
        {
            carryEncrypted[bitIndex] = carryBit;
            resultEncrypted[bitIndex] = FullAdder(numberAEncrypted[bitIndex], numberBEncrypted[bitIndex], carryBit);
        }

        // decrypt our result
        uint64 resultDecrypted = 0;
        for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
        {
            if (Decrypt(key, resultEncrypted[bitIndex]))
                resultDecrypted |= uint64(1) << uint64(bitIndex);
        }

        // report the results for the first iteration of the loop
        if (index == 0)
        {
            printf("Key 0x%" PRIx64 ", %" PRId64 " + %" PRId64 " in %i bits = %" PRId64 "\n", key, numberA, numberB, c_numBitsAdded, (numberA + numberB) % (1 << c_numBitsAdded));
            for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
                printf("  A[%i] = 0x%" PRIx64 " (%i err=%0.2f%%)\n", bitIndex, numberAEncrypted[bitIndex], Decrypt(key, numberAEncrypted[bitIndex]), GetErrorPercent(key, numberAEncrypted[bitIndex]));
            printf("+\n");
            for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
                printf("  B[%i] = 0x%" PRIx64 " (%i err=%0.2f%%)\n", bitIndex, numberBEncrypted[bitIndex], Decrypt(key, numberBEncrypted[bitIndex]), GetErrorPercent(key, numberBEncrypted[bitIndex]));
            printf("=\n");
            for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
                printf("  Result[%i] = 0x%" PRIx64 " (%i err=%0.2f%%)\n", bitIndex, resultEncrypted[bitIndex], Decrypt(key, resultEncrypted[bitIndex]), GetErrorPercent(key, resultEncrypted[bitIndex]));
            printf("Carry Bits =\n");
            for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
                printf("  Result[%i] = 0x%" PRIx64 " (%i err=%0.2f%%)\n", bitIndex, carryEncrypted[bitIndex], Decrypt(key, carryEncrypted[bitIndex]), GetErrorPercent(key, carryEncrypted[bitIndex]));
            printf("result decrypted = %" PRId64 "\n", resultDecrypted);
        }

        // make sure that the results match, keeping in mind that the 4 bit encryption may have rolled over
        Assert(resultDecrypted == ((numberA + numberB) % (1 << c_numBitsAdded)));
    }

    WaitForEnter();
    return 0;
}