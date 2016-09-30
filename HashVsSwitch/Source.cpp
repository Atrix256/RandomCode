#include <stdio.h>
#include <chrono>
#include "compile-time-crc32.h"
#include <unordered_map>
#include <map>
#include <string>
#include <array>

// To generate new test code, turn on GENERATE_CODE, turn off DO_TEST, run the program, then turn off GENERATE_CODE and turn on DO_TEST

// To find salt, turn off GENERATE_CODE and DO_TEST and turn on FIND_SALT. Set c_numHashBuckets to the number of buckets you want, run the program, and wait.
// Note: strange numbered buckets (prime or non powers of two) seem to find salts more easily.

// run time parameters
#define GENERATE_CODE()   1
#define DO_TEST()         1
#define FIND_SALT()       0
#define VERBOSE_SAMPLES() 0  // Turn on to show info for each sample
static const size_t c_testRepeatCount = 10000;
static const size_t c_testSamples = 50;

// code generation params
static const size_t c_numWords = 100;
static const unsigned int c_numHashBuckets = 337;
static const unsigned int c_salt = 1147287;

// 2000 buckets has a good salt of 8
// 1500 buckets has a good salt of 56
// 738 buckets has a good salt of 17
// 337 buckets has a good salt of 1147287

constexpr unsigned int crc32(const char *s, unsigned int salt = 0) {
    return crc32_rec(salt, s);
}

struct CRC32Hasher {
    std::size_t operator()(const std::string& s) const {
        return crc32(s.c_str());
    }
};

struct CRC32HasherMinimized {
    std::size_t operator()(const std::string& s) const {
        return crc32(s.c_str(), c_salt) % c_numHashBuckets;
    }
};

// collects multiple timings of a block of code and calculates the average and standard deviation (variance)
// incremental average and variance algorithm taken from: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
struct SBlockTimerAggregator {
    SBlockTimerAggregator(const char* label)
        : m_label(label)
        , m_numSamples(0)
        , m_mean(0.0f)
        , m_M2(0.0f)
    {

    }

    void AddSample (float milliseconds)
    {
        ++m_numSamples;
        float delta = milliseconds - m_mean;
        m_mean += delta / float(m_numSamples);
        m_M2 += delta * (milliseconds - m_mean);
    }

    ~SBlockTimerAggregator ()
    {
        printf("%s: avg = %0.2f ms. std dev = %0.2f ms\n", m_label, GetAverage(), GetStandardDeviation());
    }

    float GetAverage () const
    {
        return m_mean;
    }

    float GetVariance () const
    {
        // invalid!
        if (m_numSamples < 2)
            return -1.0f;
        
        return m_M2 / float (m_numSamples - 1);
    }

    float GetStandardDeviation () const
    {
        return sqrt(GetVariance());
    }

    const char* m_label;

    int         m_numSamples;
    float       m_mean;
    float       m_M2;
};

// times a block of code
struct SBlockTimer
{
    SBlockTimer(SBlockTimerAggregator& aggregator)
        : m_aggregator(aggregator)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~SBlockTimer()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        float milliseconds = seconds.count() * 1000.0f;
        m_aggregator.AddSample(milliseconds);

        #if VERBOSE_SAMPLES()
        printf("%s: %0.2f ms  (avg = %0.2f ms. std dev = %0.2f ms) \n", m_aggregator.m_label, milliseconds, m_aggregator.GetAverage(), m_aggregator.GetStandardDeviation());
        #endif
    }

    SBlockTimerAggregator&                m_aggregator;
    std::chrono::high_resolution_clock::time_point m_start;
};

void Fail()
{
    printf("\n\n!!! ERROR !!!\n\n");
    system("pause");
    exit(1);
}

// from http://www.randomwordgenerator.com/
const char* c_words[c_numWords] =
{
    "nuance",
    "vertical",
    "projection",
    "peace",
    "exile",
    "shop",
    "situation",
    "bell",
    "fade",
    "conviction",
    "cutting",
    "television",
    "shot",
    "momentum",
    "pride",
    "betray",
    "clock",
    "glide",
    "session",
    "explain",
    "abortion",
    "shake",
    "stunning",
    "neutral",
    "productive",
    "daughter",
    "pig",
    "correspond",
    "barrier",
    "perfume",
    "timber",
    "plaintiff",
    "oven",
    "depend",
    "frozen",
    "facade",
    "drawer",
    "maze",
    "information",
    "bird",
    "pole",
    "ignite",
    "suspect",
    "parameter",
    "decade",
    "reproduce",
    "describe",
    "jealous",
    "discriminate",
    "weave",
    "qualification",
    "lifestyle",
    "dairy",
    "empirical",
    "golf",
    "particle",
    "virus",
    "support",
    "noise",
    "old age",
    "spend",
    "punch",
    "deck",
    "commemorate",
    "excess",
    "recognize",
    "hay",
    "speed",
    "salt",
    "constraint",
    "bishop",
    "belief",
    "aids",
    "iron",
    "stop",
    "human body",
    "please",
    "assault",
    "arch",
    "ethics",
    "nap",
    "winner",
    "magnitude",
    "candidate",
    "compromise",
    "enhance",
    "purpose",
    "tradition",
    "album",
    "retire",
    "anticipation",
    "log",
    "talented",
    "refund",
    "resort",
    "observation",
    "lake",
    "siege",
    "balance",
    "tell"
};

void GenerateCode()
{
    // open file / setup
    FILE *file = fopen("Tests.h", "w+t");
    if (!file)
        return;
    #define WRITECODE fprintf(file,

    // Make g_unorderedMap
    WRITECODE "static const std::unordered_map<std::string, int> g_unorderedMap = {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "    { \"%s\", %i },\n", c_words[i], i+1);
    WRITECODE "};\n\n");

    // Make g_unorderedMapCRC32
    WRITECODE "static const std::unordered_map<std::string, int, CRC32Hasher> g_unorderedMapCRC32 = {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "    { \"%s\", %i },\n", c_words[i], i + 1);
    WRITECODE "};\n\n");

    // Make g_unorderedMapCRC32Minimized
    WRITECODE "static const std::unordered_map<std::string, int, CRC32HasherMinimized> g_unorderedMapCRC32Minimized = {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "    { \"%s\", %i },\n", c_words[i], i + 1);
    WRITECODE "};\n\n");

    // Make g_map
    WRITECODE "static const std::map<std::string, int> g_map = {\n");
    for (int i = 0; i < c_numWords; ++i) 
        WRITECODE "    { \"%s\", %i },\n", c_words[i], i + 1);
    WRITECODE "};\n\n");

    // make SwitchValue() function
    WRITECODE "int SwitchValue (const char* s) {\n");
    WRITECODE "    switch(crc32(s)) {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "        case crc32(\"%s\"): return %i;\n", c_words[i], i+1);
    WRITECODE "    }\n    Fail(); return -1;\n}\n\n");

    // make SwitchValueValidate() function
    WRITECODE "int SwitchValueValidate (const char* s) {\n");
    WRITECODE "    switch(crc32(s)) {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "        case crc32(\"%s\"): if (!strcmp(s, \"%s\")) return %i; else break;\n", c_words[i], c_words[i], i+1);
    WRITECODE "    }\n    Fail(); return -1;\n}\n\n");

    // make SwitchValueMinimized() function
    WRITECODE "int SwitchValueMinimized (const char* s) {\n");
    WRITECODE "    switch(crc32(s, c_salt) %% c_numHashBuckets) {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "        case (crc32(\"%s\", c_salt) %% c_numHashBuckets): return %i;\n", c_words[i], i+1);
    WRITECODE "    }\n    Fail(); return -1;\n}\n\n");

    // make SwitchValueMinimizedValidate() function
    WRITECODE "int SwitchValueMinimizedValidate (const char* s) {\n");
    WRITECODE "    switch(crc32(s, c_salt) %% c_numHashBuckets) {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "        case (crc32(\"%s\", c_salt) %% c_numHashBuckets): if (!strcmp(s, \"%s\")) return %i; else break;\n", c_words[i], c_words[i], i + 1);
    WRITECODE "    }\n    Fail(); return -1;\n}\n\n");

    // make array based version of SwitchValueMinimized() function
    WRITECODE "int g_SwitchValueMinimizedArray[c_numHashBuckets] = {\n");
    for (int i = 0; i < c_numHashBuckets; ++i) {
        bool winnerFound = false;
        for (int j = 0; j < c_numWords; ++j) {
            if ((crc32(c_words[j], c_salt) % c_numHashBuckets) == i) {
                winnerFound = true;
                WRITECODE "    %i, \n", j + 1);
                break;
            }
        }
        if (!winnerFound)
            WRITECODE "    0, // Unused bucket, wasted memory\n");
    }
    WRITECODE "};\n\n");

    // make array based version of SwitchValueMinimizedValidate() function
    WRITECODE "struct SValidate {\n");
    WRITECODE "    const char* m_string;\n");
    WRITECODE "    int         m_value;\n");
    WRITECODE "};\n\n");
    WRITECODE "SValidate g_SwitchValueMinimizedArrayValidate[c_numHashBuckets] = {\n");
    for (int i = 0; i < c_numHashBuckets; ++i) {
        bool winnerFound = false;
        for (int j = 0; j < c_numWords; ++j) {
            if ((crc32(c_words[j], c_salt) % c_numHashBuckets) == i) {
                winnerFound = true;
                WRITECODE "    {\"%s\", %i}, \n", c_words[j], j + 1);
                break;
            }
        }
        if (!winnerFound)
            WRITECODE "    {nullptr, 0}, // Unused bucket, wasted memory\n");
    }
    WRITECODE "};\n\n");

    // make brute force by starting letter function
    WRITECODE "int BruteForceByStartingLetter (const char* s) {\n");
    WRITECODE "    switch(*s) {\n");
    for (char c = 'a'; c <= 'z'; ++c) {
        WRITECODE "        case \'%c\': {\n", c);
        bool firstWord = true;
        for (int i = 0; i < c_numWords; ++i) {
            if (c_words[i][0] == c) {
                if (firstWord) {
                    WRITECODE "            if");
                }
                else {
                    WRITECODE "            else if");
                }
                WRITECODE "(!strcmp(&s[1], \"%s\"))\n", &(c_words[i][1]));
                WRITECODE "                return %i;\n", i);
                firstWord = false;
            }
        }
        WRITECODE "            break;\n");
        WRITECODE "        }\n");
    }
    WRITECODE "        default: Fail(); return -1;\n");
    WRITECODE "    }\n");
    WRITECODE "    Fail(); return -1;");
    WRITECODE "}\n\n");

    // make brute force function
    WRITECODE "int BruteForce (const char* s) {\n");
    WRITECODE "    if (!strcmp(s, \"");
    WRITECODE c_words[0]);
    WRITECODE "\")) return 1;\n");
    for (int i = 1; i < c_numWords; ++i)
        WRITECODE "    else if (!strcmp(s, \"%s\")) return %i;\n", c_words[i], i+1);
    WRITECODE "    Fail(); return -1;\n};\n\n");

    // close file / shutdown
    fclose(file);
    #undef WRITECODE
}

#if DO_TEST()
#include "Tests.h"

void DoTest()
{
    int sum = 0;

    {
        printf("In Order:\n");

        // std::map
        {
            SBlockTimerAggregator timerAgg("    std::map");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        auto it = g_map.find(c_words[i]);
                        if (it != g_map.end())
                            sum += (*it).second;
                    }
                }
            }
        }

        // std::unordered_map default hash
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map default hash");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        auto it = g_unorderedMap.find(c_words[i]);
                        if (it != g_unorderedMap.end())
                            sum += (*it).second;
                    }
                }
            }
        }

        // std::unordered_map crc32
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map crc32");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        auto it = g_unorderedMapCRC32.find(c_words[i]);
                        if (it != g_unorderedMapCRC32.end())
                            sum += (*it).second;
                    }
                }
            }
        }

        // std::unordered_map crc32 minimized
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map crc32 minimized");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        auto it = g_unorderedMapCRC32Minimized.find(c_words[i]);
                        if (it != g_unorderedMapCRC32Minimized.end())
                            sum += (*it).second;
                    }
                }
            }
        }

        // SwitchValue()
        {
            SBlockTimerAggregator timerAgg("    SwitchValue()");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        sum += SwitchValue(c_words[i]);
                    }
                }
            }
        }

        // SwitchValueValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueValidate()");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        sum += SwitchValueValidate(c_words[i]);
                    }
                }
            }
        }

        // SwitchValueMinimized()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimized()");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        sum += SwitchValueMinimized(c_words[i]);
                    }
                }
            }
        }

        // SwitchValueMinimizedValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimizedValidate()");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        sum += SwitchValueMinimizedValidate(c_words[i]);
                    }
                }
            }
        }

        // g_SwitchValueMinimizedArray
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArray");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        sum += g_SwitchValueMinimizedArray[crc32(c_words[i], c_salt) % c_numHashBuckets];
                    }
                }
            }
        }

        // g_SwitchValueMinimizedArrayValidate
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArrayValidate");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        SValidate& item = g_SwitchValueMinimizedArrayValidate[crc32(c_words[i], c_salt) % c_numHashBuckets];
                        if (!strcmp(c_words[i], item.m_string))
                            sum += item.m_value;
                        else
                            Fail();
                    }
                }
            }
        }

        // BruteForceByStartingLetter()
        {
            SBlockTimerAggregator timerAgg("    BruteForceByStartingLetter()");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        sum += BruteForceByStartingLetter(c_words[i]);
                    }
                }
            }
        }

        // BruteForce()
        {
            SBlockTimerAggregator timerAgg("    BruteForce()");
            for (size_t sample = 0; sample < c_testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < c_testRepeatCount; ++times) {
                    for (size_t i = 0; i < c_numWords; ++i) {
                        sum += BruteForce(c_words[i]);
                    }
                }
            }
        }
    }

    system("pause");
    printf("sum = %i\n", sum); // To keep sum and everything used to calculate it from getting optimized away.
}
#endif

bool SaltCausesCollision (unsigned int salt) {
    std::array<bool, c_numHashBuckets> collisions;
    collisions.fill(false);
    for (size_t i = 0; i < c_numWords; ++i) {
        size_t hash = (crc32(c_words[i], salt) % c_numHashBuckets);
        if (collisions[hash])
            return true;
        collisions[hash] = true;
    }
    return false;
}

void FindSalt () {
    for (unsigned int salt = 1; salt != 0; ++salt) {
        if (!SaltCausesCollision(salt)) {
            printf("Salt = %u\n", salt);
            system("pause");
            return;
        }
    }

    printf("No salt found!!\n");
    system("pause");
}

int main(int argc, char** argv) {

    #if GENERATE_CODE()
        GenerateCode();
    #endif

    #if DO_TEST()
        DoTest();
    #endif

    #if FIND_SALT()
        FindSalt();
    #endif

    return 0;
}

/*

TODO:

* try __assume in switches and time that?

* align the output for easier reading, like we do in last post

* also figure out if the high resolution clock is QPC or what, and report it in testing details

* do a series of tests with pre-hashed keys for faster lookup (like, what you would do with cooked data)

* make it show variance like last post

! Make it generate Tests.cpp as a standalone test, instead of making tests.h and relying on things from source.cpp!
 * could make it generate code every time, but only re-write code if it isn't different.

? do we need to shuffle the order that we ask for the strings in? I think so.
 * maybe one test shuffled, one in order? report timings differently
 * this to tease out cache coherency issues

? can we make the # of buckets smaller? if so, the array will be smaller.
 * brute force try it before getting timings
 ! maybe the code generator could try and find salt, or abort if there is a collision with the salt?
 * make the "FindSalt" function and use it

? maybe run the test multiple times and report average and variance

BLOG:
* Explain what is being tested
* Explain testing procedures
* NOTE: std::map doesn't use hashing, it has a comparator function.  Only including it for timing comparisongs
* Talk about how chance of finding good hash salt are affected by bucket size and number of items in the buckets.  make a graph if you can!
* debug/release x86/x64 timing
* compare timings of what we see now vs last post.  understand and explain any discrepancies.
 * mention that the timings were a bit erratic even at 100k test runs
* NOTE: the switching on first letter function only does strcmp on the rest of the string after the first letter, to be slightly more efficient
* NOTE: brute force my starting letter looks pretty good, but that isn't a nail in the coffin for "code is faster than data".  This shows that this construct is faster than a general hash table by a decent amount.
 * could generate code based on heuristics similarly to how the compiler decides how to implement a switch statement.
 * could switch on multiple first letters if you had a lot of items.  switch(first four letters) for instance.
* NOTE: could look for a hash that is cheaper at runtime but still has no collisions.  The default hash seems faster for example.  This would help the runtime hashing we have to do before lookup.
* NOTE: could have the generator program generate different code and then profile the code it generated to decide which is fastest.
? mention the FindSalt function?
* NOTE: hash quality not important. speed and no collisions for given input only matters.
* info about switch statements: http://www.codeproject.com/Articles/100473/Something-You-May-Not-Know-About-the-Switch-Statem

Tests:
    + map
    + unordered map with default hash function
    + unordered map with crc32 function
    + unordered map with crc32 minimized function
    + Compile time hash switch function
    + Compile time hash switch function minimized
    + array based solution, instead of switch based
    + brute force (if !strcmp, else if !strcmp)
    + switch on first character, then brute force in those buckets
    + checked version of switch (1 strcmp)
    + checked version of switch minimized (1 strcmp)
    + checked version of array switch minimized (1 strcmp)

UP NEXT:
* Trie? maybe some other similar data structures?
 
*/