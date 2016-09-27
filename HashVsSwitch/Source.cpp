#include <stdio.h>
#include <chrono>
#include "compile-time-crc32.h"
#include <unordered_map>
#include <map>
#include <string>

// To generate new test code, turn on GENERATE_CODE, turn off DO_TEST, run the program, then turn off GENERATE_CODE and turn on DO_TEST

// run time parameters
#define GENERATE_CODE() 0
#define DO_TEST()       1
static const size_t c_testRepeatCount = 100000;

// code generation params
static const size_t c_numWords = 100;
static const unsigned int c_numHashBuckets = 4096;
static const unsigned int c_salt = 1339;

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

struct SBlockTimer
{
    SBlockTimer(const char* label)
        : m_label(label)
    {
        std::chrono::high_resolution_clock::now();
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~SBlockTimer()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        printf("%s: %0.2f ms\n", m_label, seconds.count()*1000.0f);
    }

    std::chrono::high_resolution_clock::time_point m_start;
    const char* m_label;
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
    "AIDS",
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
    char buffer[256];

    // Make g_unorderedMap
    WRITECODE "static const std::unordered_map<std::string, int> g_unorderedMap = {\n");
    for (int i = 0; i < c_numWords; ++i) {
        WRITECODE "    { \"");
        WRITECODE c_words[i]);
        WRITECODE "\", ");

        sprintf(buffer, "%i", i+1);
        WRITECODE buffer);

        WRITECODE " },\n");
    }
    WRITECODE "};\n\n");

    // Make g_unorderedMapCRC32
    WRITECODE "static const std::unordered_map<std::string, int, CRC32Hasher> g_unorderedMapCRC32 = {\n");
    for (int i = 0; i < c_numWords; ++i) {
        WRITECODE "    { \"");
        WRITECODE c_words[i]);
        WRITECODE "\", ");

        sprintf(buffer, "%i", i + 1);
        WRITECODE buffer);

        WRITECODE " },\n");
    }
    WRITECODE "};\n\n");

    // Make g_unorderedMapCRC32Minimized
    WRITECODE "static const std::unordered_map<std::string, int, CRC32HasherMinimized> g_unorderedMapCRC32Minimized = {\n");
    for (int i = 0; i < c_numWords; ++i) {
        WRITECODE "    { \"");
        WRITECODE c_words[i]);
        WRITECODE "\", ");

        sprintf(buffer, "%i", i + 1);
        WRITECODE buffer);

        WRITECODE " },\n");
    }
    WRITECODE "};\n\n");

    // Make g_map
    WRITECODE "static const std::map<std::string, int> g_map = {\n");
    for (int i = 0; i < c_numWords; ++i) {
        WRITECODE "    { \"");
        WRITECODE c_words[i]);
        WRITECODE "\", ");

        sprintf(buffer, "%i", i+1);
        WRITECODE buffer);

        WRITECODE " },\n");
    }
    WRITECODE "};\n\n");

    // make SwitchValue() function
    WRITECODE "int SwitchValue (const char* s) {\n");
    WRITECODE "    switch(crc32(s)) {\n");
    for (int i = 0; i < c_numWords; ++i) {
        WRITECODE "        case crc32(\"");
        WRITECODE c_words[i]);
        WRITECODE "\"): return ");
        sprintf(buffer, "%i", i + 1);
        WRITECODE buffer);
        WRITECODE ";\n");
    }
    WRITECODE "    }\n    ((int*)0)[0] = 0; return -1;\n}\n\n");

    // make SwitchValueMinimized() function
    WRITECODE "int SwitchValueMinimized (const char* s) {\n");
    WRITECODE "    switch(crc32(s, c_salt) %% c_numHashBuckets) {\n");
    for (int i = 0; i < c_numWords; ++i) {
        WRITECODE "        case (crc32(\"");
        WRITECODE c_words[i]);
        WRITECODE "\", c_salt) %% c_numHashBuckets): return ");
        sprintf(buffer, "%i", i + 1);
        WRITECODE buffer);
        WRITECODE ";\n");
    }
    WRITECODE "    }\n    ((int*)0)[0] = 0; return -1;\n}\n\n");

    // make array based version of SwitchValueMinimized() function
    WRITECODE "int g_SwitchValueMinimizedArray[c_numHashBuckets] = {\n");
    for (int i = 0; i < c_numHashBuckets; ++i) {
        bool winnerFound = false;
        for (int j = 0; j < c_numWords; ++j) {
            if ((crc32(c_words[j], c_salt) % c_numHashBuckets) == i) {
                winnerFound = true;
                WRITECODE "    ");
                sprintf(buffer, "%i", j+1);
                WRITECODE buffer);
                WRITECODE ",\n");
                break;
            }
        }
        if (!winnerFound) {
            WRITECODE "    0, // Unused bucket\n");
        }
    }
    WRITECODE "};\n\n");

    // make brute force function
    WRITECODE "int BruteForce (const char* s) {\n");
    WRITECODE "    if (!stricmp(s, \"");
    WRITECODE c_words[0]);
    WRITECODE "\")) return 1;\n");
    for (int i = 1; i < c_numWords; ++i) {
        WRITECODE "    else if (!stricmp(s, \"");
        WRITECODE c_words[i]);
        WRITECODE "\")) return ");
        sprintf(buffer, "%i", i + 1);
        WRITECODE buffer);
        WRITECODE "; \n");
    }
    WRITECODE "    ((int*)0)[0] = 0; return -1;\n};\n\n");

    // close file / shutdown
    fclose(file);
    #undef WRITECODE
}

#if DO_TEST()
#include "Tests.h"

void DoTest()
{
    int sum = 0;

    // std::map
    {
        SBlockTimer timer("std::map");
        for (size_t times = 0; times < c_testRepeatCount; ++times) {
            for (size_t i = 0; i < c_numWords; ++i) {
                auto it = g_map.find(c_words[i]);
                if (it != g_map.end())
                    sum += (*it).second;
            }
        }
    }

    // std::unordered_map default hash
    {
        SBlockTimer timer("std::unordered_map default hash");
        for (size_t times = 0; times < c_testRepeatCount; ++times) {
            for (size_t i = 0; i < c_numWords; ++i) {
                auto it = g_unorderedMap.find(c_words[i]);
                if (it != g_unorderedMap.end())
                    sum += (*it).second;
            }
        }
    }

    // std::unordered_map crc32
    {
        SBlockTimer timer("std::unordered_map crc32");
        for (size_t times = 0; times < c_testRepeatCount; ++times) {
            for (size_t i = 0; i < c_numWords; ++i) {
                auto it = g_unorderedMapCRC32.find(c_words[i]);
                if (it != g_unorderedMapCRC32.end())
                    sum += (*it).second;
            }
        }
    }

    // std::unordered_map crc32 minimized
    {
        SBlockTimer timer("std::unordered_map crc32 minimized");
        for (size_t times = 0; times < c_testRepeatCount; ++times) {
            for (size_t i = 0; i < c_numWords; ++i) {
                auto it = g_unorderedMapCRC32Minimized.find(c_words[i]);
                if (it != g_unorderedMapCRC32Minimized.end())
                    sum += (*it).second;
            }
        }
    }

    // SwitchValue()
    {
        SBlockTimer timer("SwitchValue()");
        for (size_t times = 0; times < c_testRepeatCount; ++times) {
            for (size_t i = 0; i < c_numWords; ++i) {
                sum += SwitchValue(c_words[i]);
            }
        }
    }

    // SwitchValueMinimized()
    {
        SBlockTimer timer("SwitchValueMinimized()");
        for (size_t times = 0; times < c_testRepeatCount; ++times) {
            for (size_t i = 0; i < c_numWords; ++i) {
                sum += SwitchValueMinimized(c_words[i]);
            }
        }
    }

    // g_SwitchValueMinimizedArray
    {
        SBlockTimer timer("g_SwitchValueMinimizedArray");
        for (size_t times = 0; times < c_testRepeatCount; ++times) {
            for (size_t i = 0; i < c_numWords; ++i) {
                sum += g_SwitchValueMinimizedArray[crc32(c_words[i], c_salt) % c_numHashBuckets];
            }
        }
    }

    // BruteForce()
    {
        SBlockTimer timer("BruteForce()");
        for (size_t times = 0; times < c_testRepeatCount; ++times) {
            for (size_t i = 0; i < c_numWords; ++i) {
                sum += BruteForce(c_words[i]);
            }
        }
    }

    system("pause");
    printf("sum = %i", sum); // To keep sum and everything used to calculate it from getting optimized away.
}
#endif

int main(int argc, char** argv) {

    #if GENERATE_CODE()
        GenerateCode();
    #endif

    #if DO_TEST()
        DoTest();
    #endif

    return 0;
}

/*

TODO:

Test the following:
 + map
 + unordered map with default hash function
 + unordered map with crc32 function
 + unordered map with crc32 minimized function
 + Compile time hash switch function
 + Compile time hash switch function minimized
 + array based solution, instead of switch based
 + brute force (if !strcmp, else if !strcmp)

 - switch on first character, then brute force in those buckets
 - checked version of switch (1 stricmp)
 - checked version of switch minimized (1 stricmp)
 - checked version of array switch minimized (1 stricmp)

? do we need to shuffle the order that we ask for the strings in? I think so.
 * maybe one test shuffled, one in order? report timings differently

? can we make the # of buckets smaller? if so, the array will be smaller!
 * brute force try it before getting timings

BLOG:
* Explain what is being tested
* Explain testing procedures
* NOTE: std::map doesn't use hashing, it has a comparator function.  Only including it for timing comparisongs
* Talk about how chance of finding good hash salt are affected by bucket size and number of items in the buckets.  make a graph if you can!
* debug/release x86/x64 timing
* compare timings of what we see now vs last post.  understand and explain any discrepancies.
 * mention that the timings were a bit erratic even at 100k test runs

UP NEXT:
* Trie? maybe some other similar data structures?

*/