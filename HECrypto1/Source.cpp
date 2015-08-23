#include <stdio.h>
#include <random>
#include <array>
#include <stdint.h>

#define Assert(x) if (!(x)) ((int*)nullptr)[0] = 0;

// how many bytes there are in the secret key. Increase for more security
const size_t c_keyBytes = 1;

//=================================================================================
void WaitForEnter ()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
// Replace with something crypto secure if desired
uint8_t Random_uint8_t ()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

int RandomInt (int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

//=================================================================================
template <size_t KEYBYTES>
class CFHEEncryptedBit
{
public:

    // constructor for creating an unencrypted value
    CFHEEncryptedBit (bool value)
    {
        uint8_t = value ? 0xFF : 0;
        for (uint8_t &v : m_value)
            v = value;
    }

    // logical operators
    void XOR (const CFHEEncryptedBit<KEYBYTES>& rhs)
    {
        for (size_t i = 0; i < KEYBYTES; ++i)
            m_value[i] = m_value[i] ^ rhs.m_value[i];
    }

    void AND (const CFHEEncryptedBit<KEYBYTES>& rhs)
    {
        for (size_t i = 0; i < KEYBYTES; ++i)
            m_value[i] = m_value[i] & rhs.m_value[i];
    }

    void NOT ()
    {
        for (uint8_t &v : m_value)
            v = ~v;
    }

private:
    template <size_t KEYBYTES>
    friend class CFHEPrivateKey;

    CFHEEncryptedBit (const std::array<uint8_t, KEYBYTES>& key, bool value)
    {
        // generate the starting vector
        for (uint8_t &v: m_value)
            v = Random_uint8_t();

        // if the vector comes up with the wrong answer, flip a bit that corresponds
        // to a 1 in the key, to flip our result
        if (Decrypt(key) != value)
        {
            size_t byteIndex;
            uint8_t bitMask;
            RandomKeyBitTrue(key, byteIndex, bitMask);
            m_value[byteIndex] = m_value[byteIndex] ^ bitMask;
        }

        // make sure we encrypted it correctly
        Assert(Decrypt(key) == value);
    }

    bool Decrypt (const std::array<uint8_t, KEYBYTES>& key) const
    {
        // for each bit where the key and the random bits match, flip the return value
        bool ret = false;
        for (size_t i = 0; i < KEYBYTES; ++i)
        {
            uint8_t result = m_value[i] & key[i];

            while (result)
            {
                if (result & 1)
                    ret = !ret;
                result = result >> 1;
            }
        }
        return ret;
    }

    static void RandomKeyBitTrue (const std::array<uint8_t, KEYBYTES>& key, size_t& byteIndex, uint8_t& bitMask)
    {
        // get a byte index that has bits set in it
        byteIndex = RandomInt(0, KEYBYTES - 1);
        while (key[byteIndex] == 0)
            byteIndex = (byteIndex + 1) % KEYBYTES;

        // get a bit index that is set
        size_t bitIndex = RandomInt(0, 7);
        bitMask = 1 << bitIndex;
        while ((key[byteIndex] & bitMask) == 0)
        {
            bitIndex = (bitIndex + 1) & 7;
            bitMask = 1 << bitIndex;
        }
    }

    std::array<uint8_t, KEYBYTES>   m_value;
};

//=================================================================================
template <size_t KEYBYTES>
class CFHEPrivateKey
{
public:
    CFHEPrivateKey ()
    {
        // make sure there's at least one bit set to true
        bool hasAnySet = false;
        do
        {
            for (uint8_t &v : m_key)
            {
                v = Random_uint8_t();
                if (v)
                    hasAnySet = true;
            }
        }
        while (!hasAnySet);

        // TODO: remove!
        m_key[0] = 15;
    }

    CFHEEncryptedBit<KEYBYTES> EncryptBit (bool value)
    {
        return CFHEEncryptedBit<KEYBYTES>(m_key, value);
    }

    bool DecryptBit (const CFHEEncryptedBit<KEYBYTES>& value)
    {
        return value.Decrypt(m_key);
    }

private:
    std::array<uint8_t, KEYBYTES>   m_key;
};

typedef CFHEPrivateKey<c_keyBytes>      TPrivateKey;
typedef CFHEEncryptedBit<c_keyBytes>    TEncryptedBit;

//=================================================================================
int main (int argc, char **argv)
{
    /*
    TPrivateKey privateKey;
    TEncryptedBit falseBit = privateKey.EncryptBit(false);
    TEncryptedBit trueBit = privateKey.EncryptBit(true);
    TEncryptedBit trueBit2 = privateKey.EncryptBit(true);
    */

    for (int i = 0; i < 100000; ++i)
    {
        TPrivateKey privateKey;
        TEncryptedBit falseBit = privateKey.EncryptBit(false);
        TEncryptedBit trueBit = privateKey.EncryptBit(true);
        TEncryptedBit trueBit2 = privateKey.EncryptBit(true);

        falseBit.XOR(trueBit);
        Assert(privateKey.DecryptBit(falseBit) == true);
        falseBit.XOR(trueBit2);
        Assert(privateKey.DecryptBit(falseBit) == false);
        falseBit.NOT();
        Assert(privateKey.DecryptBit(falseBit) == true);
    }

    WaitForEnter();
    return 0;
}

/*
TODO:
* AND isn't always working, think about that a bit!
* NOT is also having a problem... wtf?
* i think maybe we need the X_i_j like described in that paper.


* make it so you have a templated encrypted object.  it does sizeof(type)*8 to figure out how many encrypted bits it needs etc.
* unit tests?
* make it do useful work!
* do some profiling to show how fast it is?
* make something that uses all features


BLOG:
* talk about xor, and, not
* talk about "unencrypted" values being used in the math too
* talk about bit rotation and bit shifting
* mention that it's turing complete
* basic examples with small keys
* mention that this is FHE but is insecure.  Same general idea as first FHE paper
* link to the FHE paper, and the other stuff? or wait til doing the "real" thing?
* figure out how this is insecure (gaussian elimination?)
 * probably best for part 2
 * mention it on blog though
* mention how you did the random fix up, to help keep a constant operating time

*/