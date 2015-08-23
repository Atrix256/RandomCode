#include <stdio.h>
#include <random>
#include <array>

#define Assert(x) if (!x) ((int*)nullptr)[0] = 0;

// how many bits there are in the secret key. Increase for more security
const size_t c_keyLength = 8;

// determines the epsilon by making sure this many operations can be preformed
// without the noise getting larger than 0.5
const size_t c_numOperations = 16;

//=================================================================================
template <size_t X, size_t Y, size_t Z> using TArray3D = std::array<std::array<std::array<float, Z>, Y>, X>;

//=================================================================================
void WaitForEnter()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
// Replace these with something crypto secure if desired
float RandomFloat ()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(-1, 1);
    return float(dis(gen));
}

bool RandomBit ()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 1);
    return dis(gen) == 1;
}

int RandomInt (int min, int max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

//=================================================================================
template <size_t KEYLENGTH, size_t NUMOPERATIONS>
class CFHEEncryptedBit
{
public:

    void XOR (const CFHEEncryptedBit<KEYLENGTH, NUMOPERATIONS>& rhs)
    {
        for (size_t i = 0; i < KEYLENGTH; ++i)
            m_value[i] = std::remainder(m_value[i] + rhs.m_value[i], 2.0f);

        m_estimatedError += rhs.m_estimatedError;
    }

    void AND ()
    {

    }

    float GetEstimatedError () const
    {
        return m_estimatedError;
    }

private:
    template <size_t KEYLENGTH, size_t NUMOPERATIONS>
    friend class CFHEPrivateKey;

    CFHEEncryptedBit (const std::array<bool, KEYLENGTH>& key, bool value)
    {
        const float c_epsilon = 0.49f / float(NUMOPERATIONS);
        const float c_desiredValue = value == true ? 1.0f : 0.0f;

        m_estimatedError = c_epsilon;

        // generate the starting random float vector
        // making it go from -1 to 1, and taking the abs val of the result means that
        // values wrap around from -1 to 1 so are continuious at both 0 and 1.
        for (size_t i = 0; i < KEYLENGTH; ++i)
            m_value[i] = RandomFloat();

        // adjust the dot product to what we want it to be, but make sure that we
        // preserve noise on the result, keeping it within c_epsilon.
        float difference = c_desiredValue - DotProductModulo2(key);
        size_t index = GetRandomKeyIndexSetTrue(key);
        difference += m_value[index] * c_epsilon; // preserve noise. use the random number that was there to determine noise level!
        m_value[index] = std::remainder(m_value[index] + difference, 2.0f);

        // make sure we encrypted it correctly
        Assert(Decrypt(key) == value);
    }

    bool Decrypt (const std::array<bool, KEYLENGTH>& key, float *error = nullptr) const
    {
        float rawValue = DotProductModulo2(key);

        // Note we are only returning the error for demonstration purposes. Doing this in a real setup would
        // damage your security!
        if (error != nullptr)
            *error = abs(rawValue - round(rawValue));

        float value = abs(round(rawValue));
        if (value == 1.0f)
            return true;
        else
            return false;
    }

    float DotProductModulo2 (const std::array<bool, KEYLENGTH>& key) const
    {
        float sum = 0.0f;
        for (size_t i = 0; i < KEYLENGTH; ++i)
        {
            if (key[i])
                sum += m_value[i];
        }
        return std::remainder(sum, 2.0f);
    }

    size_t GetRandomKeyIndexSetTrue (const std::array<bool, KEYLENGTH>& key) const
    {
        size_t index = RandomInt(0, KEYLENGTH-1);
        while (!key[index])
            index = (index + 1) % KEYLENGTH;
        return index;
    }

    std::array<float, KEYLENGTH>        m_value;

    // not needed for functionality, just here for demonstration purposes
    float                               m_estimatedError;
};

//=================================================================================
template <size_t KEYLENGTH, size_t NUMOPERATIONS>
class CFHEPrivateKey
{
public:
    CFHEPrivateKey ()
    {
        // make sure there's at least one bit set to true
        bool hasAnySet = false;
        do
        {
            for (size_t i = 0; i < KEYLENGTH; ++i)
            {
                m_key[i] = RandomBit();
                if (m_key[i])
                    hasAnySet = true;
            }
        }
        while (!hasAnySet);

        // Calculate the multiplication helper
        CalculateMultiplicationHelper();
    }

    CFHEEncryptedBit<KEYLENGTH, NUMOPERATIONS> EncryptBit (bool value)
    {
        return CFHEEncryptedBit<KEYLENGTH, NUMOPERATIONS>(m_key, value);
    }

    bool DecryptBit (const CFHEEncryptedBit<KEYLENGTH, NUMOPERATIONS>& value)
    {
        return value.Decrypt(m_key);
    }

    bool DecryptBit(const CFHEEncryptedBit<KEYLENGTH, NUMOPERATIONS>& value, float &error)
    {
        return value.Decrypt(m_key, &error);
    }

    const TArray3D<KEYLENGTH, KEYLENGTH, KEYLENGTH>& GetMultiplicationHelper() const
    {
        return m_multiplicationHelper;
    }

private:
    void CalculateMultiplicationHelper()
    {
        for (size_t i = 0; i < KEYLENGTH; ++i)
        {
            for (size_t j = 0; j < KEYLENGTH; ++j)
            {
                for (size_t k = 0; k < KEYLENGTH; ++k)
                {

                }
            }
        }
    }

    std::array<bool, KEYLENGTH>                 m_key;
    TArray3D<KEYLENGTH, KEYLENGTH, KEYLENGTH>   m_multiplicationHelper;
};

typedef CFHEPrivateKey<c_keyLength, c_numOperations> TPrivateKey;
typedef CFHEEncryptedBit<c_keyLength, c_numOperations> TEncryptedBit;

//=================================================================================
int main (int argc, char **argv)
{
    TPrivateKey privateKey;

    /*
    TEncryptedBit trueBit = privateKey.EncryptBit(true);
    TEncryptedBit falseBit = privateKey.EncryptBit(false);

    for (int i = 0; i < 100000; ++i)
    {
        trueBit = privateKey.EncryptBit(true);
        falseBit = privateKey.EncryptBit(false);

        if (privateKey.DecryptBit(trueBit) != true)
            Assert(false);

        if (privateKey.DecryptBit(falseBit) != false)
            Assert(false);
    }
    */

    TEncryptedBit trueBit = privateKey.EncryptBit(true);
    TEncryptedBit trueBit2 = privateKey.EncryptBit(true);
    TEncryptedBit falseBit = privateKey.EncryptBit(false);
    TEncryptedBit falseBit2 = privateKey.EncryptBit(false);

    falseBit.XOR(trueBit);
    float errorEstimated1 = falseBit.GetEstimatedError();
    float errorActual1;
    bool decryptedValue1 = privateKey.DecryptBit(falseBit, errorActual1);
    int ijkl = 0;

    falseBit.XOR(trueBit2);
    float errorEstimated2 = falseBit.GetEstimatedError();
    float errorActual2;
    bool decryptedValue2 = privateKey.DecryptBit(falseBit, errorActual2);
    ijkl = 0;

//    WaitForEnter();
}

/*
TODO:
* while doing operations, make it show internal values as well as estimated error? (#define to turn on verbose mode or something maybe?)
* make decryption report actual error as well as the value
* make AND work
* make XOR work
* figure out how the "correction" thing works to remove error (or figure out unleveled stuff!)
* make it keep track of error, by having an internal float it adds epsilon to etc
* template parameters for best performance?
* multi bit operations? like an N bit adder. abstracting from the basic stuff

* test doing a NOT by having m_value[index] = 1.0f - m_value[index]
* unit tests for encryption / decryption and the operations

* learn about zero error and gaussian elimination, and how error creeps in even if zero error
 * could reduce this to no error, and do bools only as an even simpler example!

* can also do bit rotation
 * not sure how to do bit shifting though, because we'd need to pad with zeros.  don't have a zero (i guess we could ask for one!)

* post:
 * mention proof of concept only
 * should read up to figure out correct security parameters etc
 * maybe 2 posts - first is simpler somewhat homomorphic encryption, second is fully? where to do more advanced circuits? probably on 1st one

*/