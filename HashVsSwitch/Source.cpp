#include <stdio.h>
#include <chrono>
#include "compile-time-crc32.h"
#include <unordered_map>
#include <map>
#include <string>
#include <array>
#include "test.h"
#include <random>
#include <algorithm>

// To generate new test code, turn on GENERATE_CODE, turn off DO_TEST, run the program, then turn off GENERATE_CODE and turn on DO_TEST

// To find salt, turn off GENERATE_CODE and DO_TEST and turn on FIND_SALT. Set c_numHashBuckets to the number of buckets you want, run the program, and wait.
// Note: strange numbered buckets (prime or non powers of two) seem to find salts more easily.

// run time parameters
#define GENERATE_CODE()   0
#define DO_TEST()         1
#define FIND_SALT()       0
bool   c_verboseSamples = false;  // Turn on to show info for each sample
size_t c_testRepeatCount = 5000;  // how many times a test is done for each timing sample
size_t c_testSamples = 50;        // how many timing samples are taken

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

#if GENERATE_CODE()

void GenerateCode()
{
    // open file / setup
    FILE *file = fopen("Tests.h", "w+t");
    if (!file)
        return;
    #define WRITECODE fprintf(file,

    WRITECODE "#include \"Test.h\"\n\n");

    // make a shuffled word list
    std::random_device rd;
    std::mt19937 g(rd());
    std::vector<size_t> shuffleOrder;
    shuffleOrder.resize(c_numWords);
    for (size_t i = 0; i < c_numWords; ++i)
        shuffleOrder[i] = i;
    std::shuffle(shuffleOrder.begin(), shuffleOrder.end(), g);
    WRITECODE "const char* c_wordsShuffled[c_numWords] = {\n");
    for (size_t i = 0; i < c_numWords; ++i)
        WRITECODE "    \"%s\",\n", c_words[shuffleOrder[i]]);
    WRITECODE "};\n\n");

    // make a shuffled index list for pre-hashed key test to use
    WRITECODE "size_t c_wordsShuffledOrder[c_numWords] = {\n");
    for (size_t i = 0; i < c_numWords; ++i)
        WRITECODE "    %zu,\n", shuffleOrder[i]);
    WRITECODE "};\n\n");

    // Make g_stringHashes
    WRITECODE "static const unsigned int g_stringHashes[%zu] = {\n", c_numWords);
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "    %u,\n", crc32(c_words[i]));
    WRITECODE "};\n\n");

    // Make g_stringHashesMinimized
    WRITECODE "static const unsigned int g_stringHashesMinimized[%zu] = {\n", c_numWords);
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "    %u,\n", crc32(c_words[i], c_salt)%c_numHashBuckets);
    WRITECODE "};\n\n");

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

    // make SwitchValue() and SwitchValueRaw() functions
    WRITECODE "inline int SwitchValueRaw (unsigned int hash) {\n");
    WRITECODE "    switch(hash) {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "        case crc32(\"%s\"): return %i;\n", c_words[i], i+1);
    WRITECODE "        default: __assume(0);\n");
    WRITECODE "    }\n}\n\n");
    WRITECODE "inline int SwitchValue (const char* s) {\n");
    WRITECODE "    return SwitchValueRaw(crc32(s));\n");
    WRITECODE "}\n\n");

    // make SwitchValueValidate() and SwitchValueValidateRaw() functions
    WRITECODE "inline int SwitchValueValidateRaw (const char* s, unsigned int hash) {\n");
    WRITECODE "    switch(hash) {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "        case crc32(\"%s\"): if (!strcmp(s, \"%s\")) return %i; else break;\n", c_words[i], c_words[i], i+1);
    WRITECODE "    }\n    return 0;\n}\n\n");
    WRITECODE "inline int SwitchValueValidate (const char* s) {\n");
    WRITECODE "    return SwitchValueValidateRaw(s, crc32(s));\n");
    WRITECODE "}\n\n");

    // make SwitchValueMinimized() and SwitchValueMinimizedRaw() functions
    WRITECODE "inline int SwitchValueMinimizedRaw (unsigned int hash) {\n");
    WRITECODE "    switch(hash) {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "        case (crc32(\"%s\", c_salt) %% c_numHashBuckets): return %i;\n", c_words[i], i+1);
    WRITECODE "        default: __assume(0);\n");
    WRITECODE "    }\n}\n\n");
    WRITECODE "inline int SwitchValueMinimized (const char* s) {\n");
    WRITECODE "    return SwitchValueMinimizedRaw(crc32(s, c_salt) %% c_numHashBuckets);\n");
    WRITECODE "}\n\n");

    // make SwitchValueMinimizedValidate() and SwitchValueMinimizedValidateRaw() functions
    WRITECODE "inline int SwitchValueMinimizedValidateRaw (const char* s, unsigned int hash) {\n");
    WRITECODE "    switch(hash) {\n");
    for (int i = 0; i < c_numWords; ++i)
        WRITECODE "        case (crc32(\"%s\", c_salt) %% c_numHashBuckets): if (!strcmp(s, \"%s\")) return %i; else break;\n", c_words[i], c_words[i], i + 1);
    WRITECODE "    }\n    return 0;\n}\n\n");
    WRITECODE "inline int SwitchValueMinimizedValidate (const char* s) {\n");
    WRITECODE "    return SwitchValueMinimizedValidateRaw(s, crc32(s, c_salt) %% c_numHashBuckets);\n");
    WRITECODE "}\n\n");

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

#endif //GENERATE_CODE()

#if DO_TEST()
#include "Tests.h"

void DoTest(size_t testSamples, size_t testRepeatCount, size_t numWords)
{
    int sum = 0;

    {
        printf("In Order:\n");
        // std::map
        {
            SBlockTimerAggregator timerAgg("    std::map                           ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        auto it = g_map.find(c_words[i]);
                        if (it != g_map.end())
                            sum += (*it).second;
                    }
                }
            }
        }
        // std::unordered_map default hash
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map default hash    ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        auto it = g_unorderedMap.find(c_words[i]);
                        if (it != g_unorderedMap.end())
                            sum += (*it).second;
                    }
                }
            }
        }
        // std::unordered_map crc32
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map crc32           ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        auto it = g_unorderedMapCRC32.find(c_words[i]);
                        if (it != g_unorderedMapCRC32.end())
                            sum += (*it).second;
                    }
                }
            }
        }
        // std::unordered_map crc32 minimized
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map crc32 minimized ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        auto it = g_unorderedMapCRC32Minimized.find(c_words[i]);
                        if (it != g_unorderedMapCRC32Minimized.end())
                            sum += (*it).second;
                    }
                }
            }
        }
        // SwitchValue()
        {
            SBlockTimerAggregator timerAgg("    SwitchValue()                      ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValue(c_words[i]);
                    }
                }
            }
        }
        // SwitchValueValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueValidate()              ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueValidate(c_words[i]);
                    }
                }
            }
        }
        // SwitchValueMinimized()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimized()             ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueMinimized(c_words[i]);
                    }
                }
            }
        }
        // SwitchValueMinimizedValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimizedValidate()     ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueMinimizedValidate(c_words[i]);
                    }
                }
            }
        }
        // g_SwitchValueMinimizedArray
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArray        ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += g_SwitchValueMinimizedArray[crc32(c_words[i], c_salt) % c_numHashBuckets];
                    }
                }
            }
        }
        // g_SwitchValueMinimizedArrayValidate
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArrayValidate");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
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
            SBlockTimerAggregator timerAgg("    BruteForceByStartingLetter()       ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += BruteForceByStartingLetter(c_words[i]);
                    }
                }
            }
        }
        // BruteForce()
        {
            SBlockTimerAggregator timerAgg("    BruteForce()                       ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += BruteForce(c_words[i]);
                    }
                }
            }
        }
    }
    // Shuffled word lookup
    {
        printf("Shuffled:\n");
        // std::map
        {
            SBlockTimerAggregator timerAgg("    std::map                           ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        auto it = g_map.find(c_wordsShuffled[i]);
                        if (it != g_map.end())
                            sum += (*it).second;
                    }
                }
            }
        }
        // std::unordered_map default hash
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map default hash    ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        auto it = g_unorderedMap.find(c_words[i]);
                        if (it != g_unorderedMap.end())
                            sum += (*it).second;
                    }
                }
            }
        }
        // std::unordered_map crc32
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map crc32           ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        auto it = g_unorderedMapCRC32.find(c_wordsShuffled[i]);
                        if (it != g_unorderedMapCRC32.end())
                            sum += (*it).second;
                    }
                }
            }
        }
        // std::unordered_map crc32 minimized
        {
            SBlockTimerAggregator timerAgg("    std::unordered_map crc32 minimized ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        auto it = g_unorderedMapCRC32Minimized.find(c_wordsShuffled[i]);
                        if (it != g_unorderedMapCRC32Minimized.end())
                            sum += (*it).second;
                    }
                }
            }
        }
        // SwitchValue()
        {
            SBlockTimerAggregator timerAgg("    SwitchValue()                      ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValue(c_wordsShuffled[i]);
                    }
                }
            }
        }
        // SwitchValueValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueValidate()              ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueValidate(c_wordsShuffled[i]);
                    }
                }
            }
        }
        // SwitchValueMinimized()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimized()             ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueMinimized(c_wordsShuffled[i]);
                    }
                }
            }
        }
        // SwitchValueMinimizedValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimizedValidate()     ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueMinimizedValidate(c_wordsShuffled[i]);
                    }
                }
            }
        }
        // g_SwitchValueMinimizedArray
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArray        ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += g_SwitchValueMinimizedArray[crc32(c_wordsShuffled[i], c_salt) % c_numHashBuckets];
                    }
                }
            }
        }
        // g_SwitchValueMinimizedArrayValidate
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArrayValidate");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        SValidate& item = g_SwitchValueMinimizedArrayValidate[crc32(c_wordsShuffled[i], c_salt) % c_numHashBuckets];
                        if (!strcmp(c_wordsShuffled[i], item.m_string))
                            sum += item.m_value;
                        else
                            Fail();
                    }
                }
            }
        }
        // BruteForceByStartingLetter()
        {
            SBlockTimerAggregator timerAgg("    BruteForceByStartingLetter()       ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += BruteForceByStartingLetter(c_wordsShuffled[i]);
                    }
                }
            }
        }
        // BruteForce()
        {
            SBlockTimerAggregator timerAgg("    BruteForce()                       ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += BruteForce(c_wordsShuffled[i]);
                    }
                }
            }
        }
    }
    // In Order Lookup by prehashed keys
    {
        printf("In Order Pre hashed keys:\n");

        // SwitchValue()
        {
            SBlockTimerAggregator timerAgg("    SwitchValue()                      ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueRaw(g_stringHashes[i]);
                    }
                }
            }
        }
        // SwitchValueValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueValidate()              ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueValidateRaw(c_words[i], g_stringHashes[i]);
                    }
                }
            }
        }
        // SwitchValueMinimized()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimized()             ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueMinimizedRaw(g_stringHashesMinimized[i]);
                    }
                }
            }
        }
        // SwitchValueMinimizedValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimizedValidate()     ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueMinimizedValidateRaw(c_words[i], g_stringHashesMinimized[i]);
                    }
                }
            }
        }
        // g_SwitchValueMinimizedArray
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArray        ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += g_SwitchValueMinimizedArray[g_stringHashesMinimized[i]];
                    }
                }
            }
        }
        // g_SwitchValueMinimizedArrayValidate
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArrayValidate");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        SValidate& item = g_SwitchValueMinimizedArrayValidate[g_stringHashesMinimized[i]];
                        if (!strcmp(c_words[i], item.m_string))
                            sum += item.m_value;
                        else
                            Fail();
                    }
                }
            }
        }
    }
    // Shuffled lookup by prehashed keys
    {
        printf("Shuffled Pre hashed keys:\n");

        // SwitchValue()
        {
            SBlockTimerAggregator timerAgg("    SwitchValue()                      ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueRaw(g_stringHashes[c_wordsShuffledOrder[i]]);
                    }
                }
            }
        }
        // SwitchValueValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueValidate()              ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueValidateRaw(c_words[i], g_stringHashes[c_wordsShuffledOrder[i]]);
                    }
                }
            }
        }
        // SwitchValueMinimized()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimized()             ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueMinimizedRaw(g_stringHashesMinimized[c_wordsShuffledOrder[i]]);
                    }
                }
            }
        }
        // SwitchValueMinimizedValidate()
        {
            SBlockTimerAggregator timerAgg("    SwitchValueMinimizedValidate()     ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += SwitchValueMinimizedValidateRaw(c_words[c_wordsShuffledOrder[i]], g_stringHashesMinimized[c_wordsShuffledOrder[i]]);
                    }
                }
            }
        }
        // g_SwitchValueMinimizedArray
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArray        ");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        sum += g_SwitchValueMinimizedArray[g_stringHashesMinimized[c_wordsShuffledOrder[i]]];
                    }
                }
            }
        }
        // g_SwitchValueMinimizedArrayValidate
        {
            SBlockTimerAggregator timerAgg("    g_SwitchValueMinimizedArrayValidate");
            for (size_t sample = 0; sample < testSamples; ++sample)
            {
                SBlockTimer timer(timerAgg);
                for (size_t times = 0; times < testRepeatCount; ++times) {
                    for (size_t i = 0; i < numWords; ++i) {
                        SValidate& item = g_SwitchValueMinimizedArrayValidate[g_stringHashesMinimized[c_wordsShuffledOrder[i]]];
                        if (!strcmp(c_words[c_wordsShuffledOrder[i]], item.m_string))
                            sum += item.m_value;
                        else
                            Fail();
                    }
                }
            }
        }
    }

    system("pause");
    printf("sum = %i\n", sum); // To keep sum and everything used to calculate it from getting optimized away.
}
#endif //DO_TEST()

#if FIND_SALT()

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

#endif // FIND_SALT()

int main(int argc, char** argv) {

    #if GENERATE_CODE()
        GenerateCode();
    #endif

    #if DO_TEST()
		size_t multiplier = argc < 10 ? 1 : argc; // to make sure it doesn't optimize the loops into not doing the full amount of work (eg. multiplying result by c_testRepeatCount)
		DoTest(c_testSamples * multiplier, c_testRepeatCount * multiplier, c_numWords * multiplier);
    #endif

    #if FIND_SALT()
        FindSalt();
    #endif

    return 0;
}
