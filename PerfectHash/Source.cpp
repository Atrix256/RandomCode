#include <vector>
#include <algorithm>
#include <assert.h>
#include <fstream>
#include <string>

unsigned int MurmurHash2(const void * key, int len, unsigned int seed);

//=================================================================================
template <typename VALUE>
class CPerfectHashTable {
public:

    typedef std::pair<std::string, VALUE> TKeyValuePair;
    typedef std::vector<TKeyValuePair> TKeyValueVector;
    struct SKeyValueVectorBucket {
        TKeyValueVector m_vector;
        size_t          m_bucketIndex;
    };
    typedef std::vector<struct SKeyValueVectorBucket> TKeyValueVectorBuckets;

    // Create the perfect hash table for the specified data items
    void Calculate (const TKeyValueVector& data) {

        // ----- Step 1: hash each data item and collect them into buckets.
        m_numItems = data.size();
        m_numBuckets = NumBucketsForSize(m_numItems);
        m_bucketMask = m_numBuckets - 1;
        m_salts.resize(m_numBuckets);
        m_data.resize(m_numItems);
        TKeyValueVectorBuckets buckets;
        buckets.resize(m_numBuckets);

        for (size_t i = 0; i < m_numBuckets; ++i)
            buckets[i].m_bucketIndex = i;

        for (const TKeyValuePair& pair : data) {
            size_t bucket = FirstHash(pair.first.c_str());
            buckets[bucket].m_vector.push_back(pair);
        }

        // ----- Step 2: sort buckets from most items to fewest items
        std::sort(
            buckets.begin(),
            buckets.end(),
            [](const SKeyValueVectorBucket& a, const SKeyValueVectorBucket& b) {
                return a.m_vector.size() > b.m_vector.size();
            }
        );

        // ----- Step 3: find salt values for each bucket such that when all items
        // are hashed with their bucket's salt value, that there are no collisions.
        // Note that we can stop when we hit a zero sized bucket since they are sorted
        // by length descending.
        std::vector<bool> slotsClaimed;
        slotsClaimed.resize(m_numItems);
        for (size_t bucketIndex = 0, bucketCount = buckets.size(); bucketIndex < bucketCount; ++bucketIndex) {
            if (buckets[bucketIndex].m_vector.size() == 0)
                break;
            FindSaltForBucket(buckets[bucketIndex], slotsClaimed);
        }
    }

    // Look up a value by key.  Get a pointer back.  null means not found.
    // You can modify data if you want, but you can't add/remove/modify keys without recalculating.
    VALUE* GetValue (const char* key) {

        // do the first hash lookup and get the salt to use
        size_t bucket = FirstHash(key);
        int salt = m_salts[bucket];

        // if the salt is negative, it's absolute value minus 1 is the index to use.
        size_t dataIndex;
        if (salt < 0)
            dataIndex = (size_t)((salt * -1) - 1);
        // else do the second hash lookup to get where the key value pair should be
        else
            dataIndex = MurmurHash2(key, strlen(key), (unsigned int)salt) % m_data.size();

        // if the keys match, we found it, else it doesn't exist in the table
        if (m_data[dataIndex].first.compare(key) == 0)
            return &m_data[dataIndex].second;
        return nullptr;
    }

private:

    unsigned int FirstHash (const char* key) {
        return MurmurHash2(key, strlen(key), 435) & m_bucketMask;
    }

    void FindSaltForBucket (const SKeyValueVectorBucket& bucket, std::vector<bool>& slotsClaimed) {

        // if the bucket size is 1, instead of looking for a salt, just encode the index to use in the salt.
        // store it as (index+1)*-1 so that we can use index 0 too.
        if (bucket.m_vector.size() == 1) {
            for (size_t i = 0, c = slotsClaimed.size(); i < c; ++i)
            {
                if (!slotsClaimed[i])
                {
                    slotsClaimed[i] = true;
                    m_salts[bucket.m_bucketIndex] = (i + 1)*-1;
                    m_data[i] = bucket.m_vector[0];
                    return;
                }
            }
            // we shouldn't ever get here
            assert(false);
        }

        // find the salt value for the items in this bucket that cause these items to take
        // only unclaimed slots
        for (int salt = 0; ; ++salt) {
            // if salt ever overflows to a negative number, that's a problem.
            assert(salt >= 0);
            std::vector<size_t> slotsThisBucket;
            bool success = std::all_of(
                bucket.m_vector.begin(),
                bucket.m_vector.end(),
                [this, &slotsThisBucket, salt, &slotsClaimed](const TKeyValuePair& keyValuePair) -> bool {
                    const char* key = keyValuePair.first.c_str();
                    unsigned int slotWanted = MurmurHash2(key, strlen(key), (unsigned int)salt) % m_numItems;
                    if (slotsClaimed[slotWanted])
                        return false;
                    if (std::find(slotsThisBucket.begin(), slotsThisBucket.end(), slotWanted) != slotsThisBucket.end())
                        return false;
                    slotsThisBucket.push_back(slotWanted);
                    return true;
                }
            );

            // When we find a salt value that fits our constraints, remember the salt
            // value and also claim all the buckets.
            if (success)
            {
                m_salts[bucket.m_bucketIndex] = salt;
                for (size_t i = 0, c = bucket.m_vector.size(); i < c; ++i)
                {
                    m_data[slotsThisBucket[i]] = bucket.m_vector[i];
                    slotsClaimed[slotsThisBucket[i]] = true;
                }
                return;
            }
        }
    }

    static size_t NumBucketsForSize (size_t size) {
        // returns how many buckets should be used for a specific number of elements.
        // Just uses the power of 2 lower than the size, or 1, whichever is bigger.
        if (!size)
            return 1;

        size_t ret = 1;
        size = size >> 1;
        while (size) {
            ret = ret << 1;
            size = size >> 1;
        }
        return ret;
    }

    // When doing a lookup, a first hash is done to find what salt to use
    // for the second hash.  This table stores the salt values for that second
    // hash.
    std::vector<int> m_salts;

    // NOTE: this stores both the key and the value.  This is to handle the
    // situation where a key is searched for which doesn't exist in the table.
    // That key will still hash to some valid index, so we need to detect that
    // it isn't the right key for that index.  If you are never going to look for
    // nonexistant keys, then you can "throw away the keys" and only store the
    // values.  That can be a nice memory savings.
    std::vector<TKeyValuePair> m_data;

    // useful values
    size_t m_numItems;
    size_t m_numBuckets;
    size_t m_bucketMask;
};

// MurmurHash code was taken from https://sites.google.com/site/murmurhash/
//=================================================================================
// MurmurHash2, by Austin Appleby
 
// Note - This code makes a few assumptions about how your machine behaves -
 
// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
 
// And it has a few limitations -
 
// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.
 
unsigned int MurmurHash2 ( const void * key, int len, unsigned int seed )
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
 
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
 
    // Initialize the hash to a 'random' value
 
    unsigned int h = seed ^ len;
 
    // Mix 4 bytes at a time into the hash
 
    const unsigned char * data = (const unsigned char *)key;
 
    while(len >= 4)
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
 
    switch(len)
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


//=================================================================================
void WaitForEnter ()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
int main (int argc, char **argv)
{
    // Read the data entries from a file.  Use the line as the key, and the line
    // number as the data.  Limit it to 100,000 entries.
    printf("Loading Data...");
    CPerfectHashTable<int> table;
    decltype(table)::TKeyValueVector data;
    std::ifstream file("words.txt");
    std::string str;
    int i = 0;
    while (std::getline(file, str) && i < 100000)
    {
        data.push_back(std::make_pair(str, i));
        ++i;
    }
    printf("Done\n\n");

    // make the perfect hash table
    printf("Generating Minimal Perfect Hash Table...");
    table.Calculate(data);
    printf("Done\n\n");

    // Verify results
    printf("Verifying results...");
    for (decltype(table)::TKeyValuePair& keyValuePair : data) {
        int* value = table.GetValue(keyValuePair.first.c_str());
        assert(value != nullptr);
        if (value == nullptr)
        {
            printf("Error, could not find data for key \"%s\"\n", keyValuePair.first.c_str());
            WaitForEnter();
            return 0;
        }
        assert(*value == keyValuePair.second);
        if (*value != keyValuePair.second)
        {
            printf("  table[\"%s\"] = %i\n", keyValuePair.first.c_str(), *value);
            printf("Incorrect value detected, should have gotten %i!\n", keyValuePair.second);
            WaitForEnter();
            return 0;
        }
    }
    printf("Done\n\n");

    WaitForEnter();
    return 0;
}

/*

Blog:
* slow to create, but fast to use
* good for when keys are static -> no new keys added.
 * you can modify the data though.
 * also, you could delete keys if you needed to.
* link to http://stevehanov.ca/blog/index.php?id=119
* uses murmurhash, but could use whatever hash function you want
* there are other ways to do minimal perfect hashing.
* could generalize the perfect hash table more. And could optimize.
* mention that time grows linearly with items
* could also add a requirement that they need to hash in sorted order too.  Would take longer to find good salt values, but should be possible.
 * or instead of sorted, any arbitrary order.
* don't need keys if you only use inputs that you know are for sure in the table.
* as a side effect, the index in the table is really a unique ID per item.
 * Handy if you want to have a unique id per item, but also you could convert to unique ID and then look up by index from then on too.
* Could also be used for a set instead of hash map. O(1) membership test.
* list of english words came from http://www-01.sil.org/linguistics/wordlists/english/
* code is wayyy faster in release than in debug.  it takes about 5 seconds for my machine to generate the table for 100,000 entries in release.  I gave up after several minutes in debug.
 * code could be optimized
* post text file with code


TODO:
* read article series you linked to, interesting sounding thing with automata

*/