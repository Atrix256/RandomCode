#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// MurmurHash code was taken from https://sites.google.com/site/murmurhash/
//-----------------------------------------------------------------------------
// MurmurHash2, by Austin Appleby
// Note - This code makes a few assumptions about how your machine behaves -
// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
// And it has a few limitations -
// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.
unsigned int MurmurHash2(const void * key, int len, unsigned int seed)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    // Initialize the hash to a 'random' value
    unsigned int h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char * data = (const unsigned char *)key;
    while (len >= 4)
    {
        unsigned int k = *(unsigned int *)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array
    switch (len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
        h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}
struct SShuffler
{
public:
    SShuffler(unsigned int numItems, unsigned int seed)
    {
        // initialize our state
        m_numItems = numItems;
        m_index = 0;
        m_seed = seed;
        // calculate next power of 4.  Needed sice the balanced feistel network needs
        // an even number of bits to work with
        m_nextPow4 = 4;
        while (m_numItems > m_nextPow4)
            m_nextPow4 *= 4;
        // find out how many bits we need to store this power of 4
        unsigned int numBits = 0;
        unsigned int mask = m_nextPow4 - 1;
        while (mask)
        {
            mask = mask >> 1;
            numBits++;
        }
        // calculate our left and right masks to split our indices for the feistel
        // network
        m_halfNumBits = numBits / 2;
        m_rightMask = (1 << m_halfNumBits) - 1;
        m_leftMask = m_rightMask << m_halfNumBits;
    }

    void Restart()
    {
        Restart(m_seed);
    }

    void Restart(unsigned int seed)
    {
        // store the seed we were given
        m_seed = seed;

        // reset our index
        m_index = 0;
    }

    // Get the next index in the shuffle.  Returning false means the shuffle
    // is finished and you should call Restart() if you want to start a new one.
    bool Shuffle(unsigned int &shuffleIndex)
    {
        // m_index is the index to start searching for the next number at
        while (m_index < m_nextPow4)
        {
            // get the next number
            shuffleIndex = NextNumber();

            // if we found a valid index, return success!
            if (shuffleIndex < m_numItems)
                return true;
        }

        // end of shuffled list if we got here.
        return false;
    }

    // Get the previous index in the shuffle.  Returning false means the shuffle
    // hit the beginning of the sequence
    bool ShuffleBackwards(unsigned int &shuffleIndex)
    {
        while (m_index > 1)
        {
            // get the last number
            shuffleIndex = LastNumber();
            // if we found a valid index, return success!
            if (shuffleIndex < m_numItems)
                return true;
        }

        // beginning of shuffled list if we got here
        return false;
    }

private:
    unsigned int NextNumber()
    {
        unsigned int ret = EncryptIndex(m_index);
        m_index++;
        return ret;
    }

    unsigned int LastNumber()
    {
        unsigned int lastIndex = m_index - 2;
        unsigned int ret = EncryptIndex(lastIndex);
        m_index--;
        return ret;
    }

    unsigned int EncryptIndex(unsigned int index)
    {
        // break our index into the left and right half
        unsigned int left = (index & m_leftMask) >> m_halfNumBits;
        unsigned int right = (index & m_rightMask);
        // do 4 feistel rounds
        for (int index = 0; index < 4; ++index)
        {
            unsigned int newLeft = right;
            unsigned int newRight = left ^ (MurmurHash2(&right, sizeof(right), m_seed) & m_rightMask);
            left = newLeft;
            right = newRight;
        }

        // put the left and right back together to form the encrypted index
        return (left << m_halfNumBits) | right;
    }

private:

    // precalculated values
    unsigned int m_nextPow4;
    unsigned int m_halfNumBits;
    unsigned int m_leftMask;
    unsigned int m_rightMask;

    // member vars
    unsigned int m_index;
    unsigned int m_seed;
    unsigned int m_numItems;

    // m_index assumptions:
    //   1) m_index is where to start looking for next valid number
    //   2) m_index - 2 is where to start looking for last valid number
};

// our songs that we are going to shuffle through
const unsigned int g_numSongs = 10;
const char *g_SongList[g_numSongs] =
{
    " 1. Head Like a Hole",
    " 2. Terrible Lie",
    " 3. Down in It",
    " 4. Sanctified",
    " 5. Something I Can Never Have",
    " 6. Kinda I Want to",
    " 7. Sin",
    " 8. That's What I Get",
    " 9. The Only Time",
    "10. Ringfinger"
};

int main(void)
{
    // create and seed our shuffler.  If two similar numbers are hashed they should give
    // very different results usually, so for a seed, we can hash the time in seconds,
    // even though that number should be really similar from run to run
    unsigned int currentTime = (unsigned int)time(NULL);
    unsigned int seed = MurmurHash2(&currentTime, sizeof(currentTime), 0x1337beef);
    SShuffler shuffler(g_numSongs, seed);

    // shuffle play the songs
    printf("Listen to Pretty Hate Machine (seed = %u)\r\n", seed);
    unsigned int shuffleIndex = 0;
    while (shuffler.Shuffle(shuffleIndex))
        printf("%s\r\n", g_SongList[shuffleIndex]);

    system("pause");
    return 0;
}