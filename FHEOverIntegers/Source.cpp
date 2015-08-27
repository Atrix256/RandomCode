/*
TODO:
* do some runs with larger key bits?
*/

// Note that this encryption scheme is insecure so please don't actually use it
// in production!  A false bit with a given key is the same value every time, and
// so is a true bit.  Also, the encrypted true bit value will always be the
// encrypted false bit plus 1.  That means if an attacker sees both an encrypted true
// bit and an encrypted false bit before any operations are done on them, they will
// be able to break the encryption!
// This is just for demonstration purposes.

#include <stdio.h>
#include <stdint.h>
#include <random>
#include <array>
#include <inttypes.h>

typedef uint64_t uint64;

// Increase this value to increase the size of the key.
// If you set it too high, operations will fail when they run out of storage space
// in the 64 bit ints.
const size_t c_numKeyBits = 4;

#define Assert(x) if (!(x)) ((int*)nullptr)[0] = 0;

//=================================================================================
// Replace with something crypto secure if desired!
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
    return RandomUint64(0, (1 << c_numKeyBits) - 1) | 1 | (1 << (c_numKeyBits - 1));
}

//=================================================================================
bool Decrypt (uint64 key, uint64 value)
{
    return ((value % key) % 2) == 1;
}

//=================================================================================
uint64 Encrypt (uint64 key, bool value)
{
    uint64 ret = key + (value ? 1 : 0);
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
uint64 FullAdder (uint64 A, uint64 B, uint64 &carryBit)
{
    // TODO: temp!  when key is 9, the sumbit operation comes up with a zero result!
    A = 10;
    B = 10;
    carryBit = 114100;

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
    printf("Verifying 10000 truth tables:\n");
    for (int index = 0; index < 10000; ++index)
    {
        // make our key and a true and false bit
        uint64 key = GenerateKey();
        uint64 falseBit = Encrypt(key, false);
        uint64 trueBit = Encrypt(key, true);

        // Verify truth tables for XOR and AND
        Assert(Decrypt(key, XOR(falseBit, falseBit)) == false);
        Assert(Decrypt(key, XOR(falseBit, trueBit )) == true );
        Assert(Decrypt(key, XOR(trueBit , falseBit)) == true );
        Assert(Decrypt(key, XOR(trueBit , trueBit )) == false);

        Assert(Decrypt(key, AND(falseBit, falseBit)) == false);
        Assert(Decrypt(key, AND(falseBit, trueBit )) == false);
        Assert(Decrypt(key, AND(trueBit , falseBit)) == false);
        Assert(Decrypt(key, AND(trueBit , trueBit )) == true );

        // report the results for the first iteration of the loop
        if (index == 0)
        {
            printf("Key 0x%" PRIx64 ", false 0x%" PRIx64 ", true 0x%" PRIx64 "\n", key, falseBit, trueBit);
            printf("  [0 xor 0] = 0   0x%" PRIx64 " xor 0x%" PRIx64 " = 0x%" PRIx64 " (%i)\n", falseBit, falseBit, XOR(falseBit, falseBit), Decrypt(key, XOR(falseBit, falseBit)) ? 1 : 0);
            printf("  [0 xor 1] = 1   0x%" PRIx64 " xor 0x%" PRIx64 " = 0x%" PRIx64 " (%i)\n", falseBit, trueBit , XOR(falseBit, trueBit ), Decrypt(key, XOR(falseBit, trueBit )) ? 1 : 0);
            printf("  [1 xor 0] = 1   0x%" PRIx64 " xor 0x%" PRIx64 " = 0x%" PRIx64 " (%i)\n", trueBit , falseBit, XOR(trueBit , falseBit), Decrypt(key, XOR(trueBit , falseBit)) ? 1 : 0);
            printf("  [1 xor 1] = 0   0x%" PRIx64 " xor 0x%" PRIx64 " = 0x%" PRIx64 " (%i)\n", trueBit , trueBit , XOR(trueBit , trueBit ), Decrypt(key, XOR(trueBit , trueBit )) ? 1 : 0);
            printf("  [0 and 0] = 0   0x%" PRIx64 " and 0x%" PRIx64 " = 0x%" PRIx64 " (%i)\n", falseBit, falseBit, AND(falseBit, falseBit), Decrypt(key, AND(falseBit, falseBit)) ? 1 : 0);
            printf("  [0 and 1] = 0   0x%" PRIx64 " and 0x%" PRIx64 " = 0x%" PRIx64 " (%i)\n", falseBit, trueBit , AND(falseBit, trueBit ), Decrypt(key, AND(falseBit, trueBit )) ? 1 : 0);
            printf("  [1 and 0] = 0   0x%" PRIx64 " and 0x%" PRIx64 " = 0x%" PRIx64 " (%i)\n", trueBit , falseBit, AND(trueBit , falseBit), Decrypt(key, AND(trueBit , falseBit)) ? 1 : 0);
            printf("  [1 and 1] = 1   0x%" PRIx64 " and 0x%" PRIx64 " = 0x%" PRIx64 " (%i)\n", trueBit , trueBit , AND(trueBit , trueBit ), Decrypt(key, AND(trueBit , trueBit )) ? 1 : 0);
        }
    }

    // Do multi bit addition as an example of using compound circuits to
    // do meaningful work.
    const size_t c_numBitsAdded = 4;
    printf("\nDoing 10000 Multibit Additions:\n");
    std::array<uint64, c_numBitsAdded> numberAEncrypted;
    std::array<uint64, c_numBitsAdded> numberBEncrypted;
    std::array<uint64, c_numBitsAdded> resultEncrypted;
    for (int index = 0; index < 10000; ++index)
    {
        // generate the numbers we want to add
        uint64 numberA = RandomUint64(0, (1 << c_numBitsAdded) - 1);
        uint64 numberB = RandomUint64(0, (1 << c_numBitsAdded) - 1);

        // generate our key
        uint64 key = GenerateKey();

        // TODO: temp!
        // TODO: when the numbers are both 15 and the key is 9, there is a problem!
        // The encrypted result has a high bit of 0!
        numberA = numberB = 15;
        key = 9;

        // encrypt our bits
        for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
        {
            numberAEncrypted[bitIndex] = Encrypt(key, (numberA & (uint64(1) << uint64(bitIndex))) != 0);
            numberBEncrypted[bitIndex] = Encrypt(key, (numberB & (uint64(1) << uint64(bitIndex))) != 0);
        }

        // do our multi bit addition!
        uint64 carryBit = Encrypt(key, false); // IMPORTANT: initialize carry bit to ENCRYPTED 0, not just 0!
        for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
            resultEncrypted[bitIndex] = FullAdder(numberAEncrypted[bitIndex], numberBEncrypted[bitIndex], carryBit);

        // decrypt our result
        uint64 resultDecrypted = 0;
        for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
        {
            if (Decrypt(key, resultEncrypted[bitIndex]))
                resultDecrypted |= uint64(1) << uint64(bitIndex);
        }

        // make sure that the results match, keeping in mind that the 4 bit encryption may have rolled over
        Assert(resultDecrypted == ((numberA + numberB) % (1 << c_numBitsAdded)));

        // report the results for the first iteration of the loop
        if (index == 0)
        {
            printf("Key 0x%" PRIx64 ", A + B, %" PRId64 " + %" PRId64 " with %i bits = %" PRId64 "\n", key, numberA, numberB, c_numBitsAdded, (numberA + numberB) % (1 << c_numBitsAdded));
            for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
                printf("  A[%i] = 0x%" PRIx64 "\n", bitIndex, numberAEncrypted[bitIndex]);
            for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
                printf("  B[%i] = 0x%" PRIx64 "\n", bitIndex, numberBEncrypted[bitIndex]);
            for (int bitIndex = 0; bitIndex < c_numBitsAdded; ++bitIndex)
                printf("  Result[%i] = 0x%" PRIx64 "\n", bitIndex, resultEncrypted[bitIndex]);
            printf("result decrypted = %" PRId64 " \n", resultDecrypted);
        }
    }

    WaitForEnter();
    return 0;
}

/*#include <stdio.h>
#include <stdint.h>
#include <random>

typedef uint64_t uint64;

const size_t c_numKeyBits = 4;

// TODO: what should these be, for security?
const uint64_t c_maxKeyMultiplier = 4;
const uint64_t c_maxNoiseValue = 4;  // if noise ever gets to key / 2, we'll get wrong answers

#define Assert(x) if (!(x)) ((int*)nullptr)[0] = 0;

//=================================================================================
// Replace with something crypto secure!
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
    // Where N is the number of bits in our key
    uint64 key = RandomUint64(0, (1 << (c_numKeyBits - 2)) - 1);
    key = (key << 1) | 1;
    key = key | (1 << (c_numKeyBits - 1));
    return key;
}

//=================================================================================
uint64 Encrypt (uint64 key, bool value)
{
    uint64 keyMultiplier = RandomUint64(0, c_maxKeyMultiplier - 1) + 1;
    uint64 noise = RandomUint64(0, c_maxNoiseValue / 2) * 2;
    return key * keyMultiplier + noise + (value ? 1 : 0);
}

//=================================================================================
bool Decrypt (uint64 key, uint64 value)
{
    return ((value % key) % 2) == 1;
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

uint64 RandomNumber(uint64 min, uint64 max)
{
    return 2;
}

//=================================================================================
int main (int argc, char **argv)
{
    uint64 N = 4;
    uint64 Key = RandomNumber(0, (1 << N) - 1) | 1 | (1 << (N - 1));

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
*/

/*
TODO: REAL first post stuff
* super super basic version. no error or multiples

Blog:
* history
* basic usage cases: financial info.  game play stuff
* link to the other HE papers?

-------------------------------------------------


TODO: First post stuff!
? what happens when we roll over, does it still work ok?
* show that it works
* show deeper circuits
? what is appropriate number of bits for security?

Blog:
* show most basic encrypt, decrypt, xor, and -> using only key + value to encrypt.
 * maybe super simple implementation with just uint32's?
* talk about why key is generated like it is
? how do we attack this
* add key multiplier and noise
* show improved security?
? how else can it be attacked?
* talk about it being turing complete
* show how AND vs XOR grow numbers and error
* mention boot strapping and modulus switching
 ? do you understand modulus switching correctly?
* mention that it can work for public / private keys too.
 ? next post how to do that perhaps?
* link to what papers? maybe just the basic paper about FHE over integers?
! WOW.  Addition litteraly adds the error, and multiplication multiplies it.  show this. by showing number % key for false and true bits and after anding and xoring.
! AND and OR didn't work when noise got too large and caused the wrong answers to come out

----- not first post!


TODO:
* this looks promising: https://github.com/coron/fhe
 * compressed public key and modulus switching!
* paper: https://eprint.iacr.org/2009/616.pdf
* make it work
* understand it's insecurities
 * what if you don't add error (remove the residue r term)? how does that change security
* generalize this stuff (AND and XOR)
* figure out bootstrapping?
* what happens when numbers roll over?
 * can we mask it (let it roll over), or do we need to do multi precision math stuff?
* do we need to do it first without bootstrapping? then an updated blog post wqith bootstrapping
* test deeper circuits, even on simplest implementation

? what is the appropriate number of bits for each thing?

? why does this work?
* how should we randomly select Q and R?  there are some recomendations, why are they secure?

? why is the key a: random odd number between 2^(N-1) and 2^N
 * why odd number:  http://crypto.stackexchange.com/questions/18454/public-key-in-fully-homomorphic-encryption-over-the-integers
  * and this: http://crypto.stackexchange.com/questions/27738/symmetric-key-in-homomorphic-encryption-over-the-integers
  * i think that answers it, but think about it / reason it out.
  * i think when the key is even, you can always see whether the key is odd or even by looking at the last bit.  when key is odd, that isn't true?
  * yes...with an even key, the encrypted bit always has the same parity as the plaintext bit, which makes the encryption rediculously easy to crack!
  * with an odd key, the parity seems randomly the same or different.
 ? why 2^(N-1) to 2^N. aka, why always have high bit set?

* do symetric key, then public / private after

! make a "tiny somewhat homomorphic" implementation using just uint32s and one operation first! then more complex stuff.

Blog:
! WOW.  Addition litteraly adds the error, and multiplication multiplies it.  show this. by showing number % key for false and true bits and after anding and xoring.
! AND and OR didn't work when noise got too large and caused the wrong answers to come out
* explain why the high and low bit of the key need to be 1

*/