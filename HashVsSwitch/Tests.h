#include "Test.h"

const char* c_wordsShuffled[c_numWords] = {
    "punch",
    "anticipation",
    "timber",
    "maze",
    "exile",
    "spend",
    "reproduce",
    "abortion",
    "conviction",
    "drawer",
    "golf",
    "decade",
    "log",
    "ignite",
    "aids",
    "human body",
    "arch",
    "virus",
    "nuance",
    "pride",
    "clock",
    "qualification",
    "compromise",
    "old age",
    "tell",
    "shot",
    "album",
    "shop",
    "stop",
    "projection",
    "lifestyle",
    "assault",
    "retire",
    "glide",
    "situation",
    "particle",
    "suspect",
    "explain",
    "jealous",
    "observation",
    "parameter",
    "recognize",
    "candidate",
    "hay",
    "bishop",
    "depend",
    "shake",
    "empirical",
    "refund",
    "commemorate",
    "describe",
    "nap",
    "resort",
    "speed",
    "dairy",
    "discriminate",
    "excess",
    "oven",
    "fade",
    "productive",
    "session",
    "talented",
    "vertical",
    "pole",
    "frozen",
    "correspond",
    "peace",
    "stunning",
    "belief",
    "deck",
    "magnitude",
    "bird",
    "bell",
    "plaintiff",
    "constraint",
    "balance",
    "siege",
    "television",
    "lake",
    "neutral",
    "information",
    "noise",
    "weave",
    "please",
    "salt",
    "purpose",
    "daughter",
    "support",
    "barrier",
    "momentum",
    "betray",
    "cutting",
    "perfume",
    "iron",
    "enhance",
    "winner",
    "pig",
    "tradition",
    "ethics",
    "facade",
};

static const unsigned int g_stringHashes[100] = {
    1323836821,
    850088970,
    2624675759,
    818418853,
    3006320226,
    1926327361,
    4124799387,
    2910896228,
    2498678083,
    376567357,
    2582578764,
    2010773322,
    1975298136,
    3307029194,
    3159670012,
    218542360,
    2275849840,
    3729971138,
    3000497493,
    503482051,
    1846800485,
    1306213536,
    213715623,
    3333932401,
    3276429049,
    4253659474,
    3437736890,
    2888405513,
    2212354601,
    1765834955,
    3757054582,
    2551027225,
    2118574069,
    2848587953,
    719550841,
    3654258635,
    2603442269,
    2544792989,
    3170981520,
    2113965805,
    601580034,
    291656586,
    4219017010,
    862026305,
    1636272128,
    2637644114,
    3453436996,
    2238531027,
    3017153966,
    3042782881,
    1201228211,
    3486293662,
    1661049943,
    3594062093,
    1998347422,
    3438919543,
    57819922,
    3801598756,
    3295977163,
    1875295169,
    3574542303,
    356284602,
    2434209492,
    2026503389,
    3304427917,
    2780335853,
    3959748121,
    922482196,
    1363197972,
    624892283,
    2748307244,
    1639729672,
    4093514003,
    2798370943,
    1743599189,
    3892795994,
    3560468918,
    2927294093,
    648610473,
    2514227384,
    2438279695,
    1120616018,
    745641552,
    3510920469,
    192628605,
    253105636,
    3658781546,
    2104957113,
    4548257,
    1800266447,
    1170633146,
    2407616040,
    244607687,
    353454596,
    2561115702,
    1359910387,
    2093691368,
    1591458837,
    3462872959,
    3639789607,
};

static const unsigned int g_stringHashesMinimized[100] = {
    149,
    123,
    37,
    298,
    317,
    109,
    296,
    279,
    293,
    65,
    119,
    64,
    21,
    195,
    117,
    197,
    150,
    277,
    55,
    44,
    135,
    175,
    181,
    180,
    168,
    307,
    153,
    260,
    4,
    286,
    133,
    159,
    76,
    212,
    139,
    74,
    29,
    318,
    14,
    141,
    225,
    115,
    267,
    26,
    283,
    311,
    128,
    254,
    229,
    75,
    289,
    194,
    78,
    287,
    68,
    274,
    127,
    156,
    56,
    80,
    114,
    121,
    73,
    187,
    325,
    164,
    303,
    278,
    282,
    235,
    223,
    15,
    18,
    89,
    154,
    301,
    280,
    174,
    312,
    35,
    200,
    291,
    63,
    16,
    258,
    126,
    48,
    155,
    184,
    321,
    2,
    207,
    268,
    143,
    62,
    20,
    222,
    43,
    158,
    103,
};

static const std::unordered_map<std::string, int> g_unorderedMap = {
    { "nuance", 1 },
    { "vertical", 2 },
    { "projection", 3 },
    { "peace", 4 },
    { "exile", 5 },
    { "shop", 6 },
    { "situation", 7 },
    { "bell", 8 },
    { "fade", 9 },
    { "conviction", 10 },
    { "cutting", 11 },
    { "television", 12 },
    { "shot", 13 },
    { "momentum", 14 },
    { "pride", 15 },
    { "betray", 16 },
    { "clock", 17 },
    { "glide", 18 },
    { "session", 19 },
    { "explain", 20 },
    { "abortion", 21 },
    { "shake", 22 },
    { "stunning", 23 },
    { "neutral", 24 },
    { "productive", 25 },
    { "daughter", 26 },
    { "pig", 27 },
    { "correspond", 28 },
    { "barrier", 29 },
    { "perfume", 30 },
    { "timber", 31 },
    { "plaintiff", 32 },
    { "oven", 33 },
    { "depend", 34 },
    { "frozen", 35 },
    { "facade", 36 },
    { "drawer", 37 },
    { "maze", 38 },
    { "information", 39 },
    { "bird", 40 },
    { "pole", 41 },
    { "ignite", 42 },
    { "suspect", 43 },
    { "parameter", 44 },
    { "decade", 45 },
    { "reproduce", 46 },
    { "describe", 47 },
    { "jealous", 48 },
    { "discriminate", 49 },
    { "weave", 50 },
    { "qualification", 51 },
    { "lifestyle", 52 },
    { "dairy", 53 },
    { "empirical", 54 },
    { "golf", 55 },
    { "particle", 56 },
    { "virus", 57 },
    { "support", 58 },
    { "noise", 59 },
    { "old age", 60 },
    { "spend", 61 },
    { "punch", 62 },
    { "deck", 63 },
    { "commemorate", 64 },
    { "excess", 65 },
    { "recognize", 66 },
    { "hay", 67 },
    { "speed", 68 },
    { "salt", 69 },
    { "constraint", 70 },
    { "bishop", 71 },
    { "belief", 72 },
    { "aids", 73 },
    { "iron", 74 },
    { "stop", 75 },
    { "human body", 76 },
    { "please", 77 },
    { "assault", 78 },
    { "arch", 79 },
    { "ethics", 80 },
    { "nap", 81 },
    { "winner", 82 },
    { "magnitude", 83 },
    { "candidate", 84 },
    { "compromise", 85 },
    { "enhance", 86 },
    { "purpose", 87 },
    { "tradition", 88 },
    { "album", 89 },
    { "retire", 90 },
    { "anticipation", 91 },
    { "log", 92 },
    { "talented", 93 },
    { "refund", 94 },
    { "resort", 95 },
    { "observation", 96 },
    { "lake", 97 },
    { "siege", 98 },
    { "balance", 99 },
    { "tell", 100 },
};

static const std::unordered_map<std::string, int, CRC32Hasher> g_unorderedMapCRC32 = {
    { "nuance", 1 },
    { "vertical", 2 },
    { "projection", 3 },
    { "peace", 4 },
    { "exile", 5 },
    { "shop", 6 },
    { "situation", 7 },
    { "bell", 8 },
    { "fade", 9 },
    { "conviction", 10 },
    { "cutting", 11 },
    { "television", 12 },
    { "shot", 13 },
    { "momentum", 14 },
    { "pride", 15 },
    { "betray", 16 },
    { "clock", 17 },
    { "glide", 18 },
    { "session", 19 },
    { "explain", 20 },
    { "abortion", 21 },
    { "shake", 22 },
    { "stunning", 23 },
    { "neutral", 24 },
    { "productive", 25 },
    { "daughter", 26 },
    { "pig", 27 },
    { "correspond", 28 },
    { "barrier", 29 },
    { "perfume", 30 },
    { "timber", 31 },
    { "plaintiff", 32 },
    { "oven", 33 },
    { "depend", 34 },
    { "frozen", 35 },
    { "facade", 36 },
    { "drawer", 37 },
    { "maze", 38 },
    { "information", 39 },
    { "bird", 40 },
    { "pole", 41 },
    { "ignite", 42 },
    { "suspect", 43 },
    { "parameter", 44 },
    { "decade", 45 },
    { "reproduce", 46 },
    { "describe", 47 },
    { "jealous", 48 },
    { "discriminate", 49 },
    { "weave", 50 },
    { "qualification", 51 },
    { "lifestyle", 52 },
    { "dairy", 53 },
    { "empirical", 54 },
    { "golf", 55 },
    { "particle", 56 },
    { "virus", 57 },
    { "support", 58 },
    { "noise", 59 },
    { "old age", 60 },
    { "spend", 61 },
    { "punch", 62 },
    { "deck", 63 },
    { "commemorate", 64 },
    { "excess", 65 },
    { "recognize", 66 },
    { "hay", 67 },
    { "speed", 68 },
    { "salt", 69 },
    { "constraint", 70 },
    { "bishop", 71 },
    { "belief", 72 },
    { "aids", 73 },
    { "iron", 74 },
    { "stop", 75 },
    { "human body", 76 },
    { "please", 77 },
    { "assault", 78 },
    { "arch", 79 },
    { "ethics", 80 },
    { "nap", 81 },
    { "winner", 82 },
    { "magnitude", 83 },
    { "candidate", 84 },
    { "compromise", 85 },
    { "enhance", 86 },
    { "purpose", 87 },
    { "tradition", 88 },
    { "album", 89 },
    { "retire", 90 },
    { "anticipation", 91 },
    { "log", 92 },
    { "talented", 93 },
    { "refund", 94 },
    { "resort", 95 },
    { "observation", 96 },
    { "lake", 97 },
    { "siege", 98 },
    { "balance", 99 },
    { "tell", 100 },
};

static const std::unordered_map<std::string, int, CRC32HasherMinimized> g_unorderedMapCRC32Minimized = {
    { "nuance", 1 },
    { "vertical", 2 },
    { "projection", 3 },
    { "peace", 4 },
    { "exile", 5 },
    { "shop", 6 },
    { "situation", 7 },
    { "bell", 8 },
    { "fade", 9 },
    { "conviction", 10 },
    { "cutting", 11 },
    { "television", 12 },
    { "shot", 13 },
    { "momentum", 14 },
    { "pride", 15 },
    { "betray", 16 },
    { "clock", 17 },
    { "glide", 18 },
    { "session", 19 },
    { "explain", 20 },
    { "abortion", 21 },
    { "shake", 22 },
    { "stunning", 23 },
    { "neutral", 24 },
    { "productive", 25 },
    { "daughter", 26 },
    { "pig", 27 },
    { "correspond", 28 },
    { "barrier", 29 },
    { "perfume", 30 },
    { "timber", 31 },
    { "plaintiff", 32 },
    { "oven", 33 },
    { "depend", 34 },
    { "frozen", 35 },
    { "facade", 36 },
    { "drawer", 37 },
    { "maze", 38 },
    { "information", 39 },
    { "bird", 40 },
    { "pole", 41 },
    { "ignite", 42 },
    { "suspect", 43 },
    { "parameter", 44 },
    { "decade", 45 },
    { "reproduce", 46 },
    { "describe", 47 },
    { "jealous", 48 },
    { "discriminate", 49 },
    { "weave", 50 },
    { "qualification", 51 },
    { "lifestyle", 52 },
    { "dairy", 53 },
    { "empirical", 54 },
    { "golf", 55 },
    { "particle", 56 },
    { "virus", 57 },
    { "support", 58 },
    { "noise", 59 },
    { "old age", 60 },
    { "spend", 61 },
    { "punch", 62 },
    { "deck", 63 },
    { "commemorate", 64 },
    { "excess", 65 },
    { "recognize", 66 },
    { "hay", 67 },
    { "speed", 68 },
    { "salt", 69 },
    { "constraint", 70 },
    { "bishop", 71 },
    { "belief", 72 },
    { "aids", 73 },
    { "iron", 74 },
    { "stop", 75 },
    { "human body", 76 },
    { "please", 77 },
    { "assault", 78 },
    { "arch", 79 },
    { "ethics", 80 },
    { "nap", 81 },
    { "winner", 82 },
    { "magnitude", 83 },
    { "candidate", 84 },
    { "compromise", 85 },
    { "enhance", 86 },
    { "purpose", 87 },
    { "tradition", 88 },
    { "album", 89 },
    { "retire", 90 },
    { "anticipation", 91 },
    { "log", 92 },
    { "talented", 93 },
    { "refund", 94 },
    { "resort", 95 },
    { "observation", 96 },
    { "lake", 97 },
    { "siege", 98 },
    { "balance", 99 },
    { "tell", 100 },
};

static const std::map<std::string, int> g_map = {
    { "nuance", 1 },
    { "vertical", 2 },
    { "projection", 3 },
    { "peace", 4 },
    { "exile", 5 },
    { "shop", 6 },
    { "situation", 7 },
    { "bell", 8 },
    { "fade", 9 },
    { "conviction", 10 },
    { "cutting", 11 },
    { "television", 12 },
    { "shot", 13 },
    { "momentum", 14 },
    { "pride", 15 },
    { "betray", 16 },
    { "clock", 17 },
    { "glide", 18 },
    { "session", 19 },
    { "explain", 20 },
    { "abortion", 21 },
    { "shake", 22 },
    { "stunning", 23 },
    { "neutral", 24 },
    { "productive", 25 },
    { "daughter", 26 },
    { "pig", 27 },
    { "correspond", 28 },
    { "barrier", 29 },
    { "perfume", 30 },
    { "timber", 31 },
    { "plaintiff", 32 },
    { "oven", 33 },
    { "depend", 34 },
    { "frozen", 35 },
    { "facade", 36 },
    { "drawer", 37 },
    { "maze", 38 },
    { "information", 39 },
    { "bird", 40 },
    { "pole", 41 },
    { "ignite", 42 },
    { "suspect", 43 },
    { "parameter", 44 },
    { "decade", 45 },
    { "reproduce", 46 },
    { "describe", 47 },
    { "jealous", 48 },
    { "discriminate", 49 },
    { "weave", 50 },
    { "qualification", 51 },
    { "lifestyle", 52 },
    { "dairy", 53 },
    { "empirical", 54 },
    { "golf", 55 },
    { "particle", 56 },
    { "virus", 57 },
    { "support", 58 },
    { "noise", 59 },
    { "old age", 60 },
    { "spend", 61 },
    { "punch", 62 },
    { "deck", 63 },
    { "commemorate", 64 },
    { "excess", 65 },
    { "recognize", 66 },
    { "hay", 67 },
    { "speed", 68 },
    { "salt", 69 },
    { "constraint", 70 },
    { "bishop", 71 },
    { "belief", 72 },
    { "aids", 73 },
    { "iron", 74 },
    { "stop", 75 },
    { "human body", 76 },
    { "please", 77 },
    { "assault", 78 },
    { "arch", 79 },
    { "ethics", 80 },
    { "nap", 81 },
    { "winner", 82 },
    { "magnitude", 83 },
    { "candidate", 84 },
    { "compromise", 85 },
    { "enhance", 86 },
    { "purpose", 87 },
    { "tradition", 88 },
    { "album", 89 },
    { "retire", 90 },
    { "anticipation", 91 },
    { "log", 92 },
    { "talented", 93 },
    { "refund", 94 },
    { "resort", 95 },
    { "observation", 96 },
    { "lake", 97 },
    { "siege", 98 },
    { "balance", 99 },
    { "tell", 100 },
};

inline int SwitchValueRaw (unsigned int hash) {
    switch(hash) {
        case crc32("nuance"): return 1;
        case crc32("vertical"): return 2;
        case crc32("projection"): return 3;
        case crc32("peace"): return 4;
        case crc32("exile"): return 5;
        case crc32("shop"): return 6;
        case crc32("situation"): return 7;
        case crc32("bell"): return 8;
        case crc32("fade"): return 9;
        case crc32("conviction"): return 10;
        case crc32("cutting"): return 11;
        case crc32("television"): return 12;
        case crc32("shot"): return 13;
        case crc32("momentum"): return 14;
        case crc32("pride"): return 15;
        case crc32("betray"): return 16;
        case crc32("clock"): return 17;
        case crc32("glide"): return 18;
        case crc32("session"): return 19;
        case crc32("explain"): return 20;
        case crc32("abortion"): return 21;
        case crc32("shake"): return 22;
        case crc32("stunning"): return 23;
        case crc32("neutral"): return 24;
        case crc32("productive"): return 25;
        case crc32("daughter"): return 26;
        case crc32("pig"): return 27;
        case crc32("correspond"): return 28;
        case crc32("barrier"): return 29;
        case crc32("perfume"): return 30;
        case crc32("timber"): return 31;
        case crc32("plaintiff"): return 32;
        case crc32("oven"): return 33;
        case crc32("depend"): return 34;
        case crc32("frozen"): return 35;
        case crc32("facade"): return 36;
        case crc32("drawer"): return 37;
        case crc32("maze"): return 38;
        case crc32("information"): return 39;
        case crc32("bird"): return 40;
        case crc32("pole"): return 41;
        case crc32("ignite"): return 42;
        case crc32("suspect"): return 43;
        case crc32("parameter"): return 44;
        case crc32("decade"): return 45;
        case crc32("reproduce"): return 46;
        case crc32("describe"): return 47;
        case crc32("jealous"): return 48;
        case crc32("discriminate"): return 49;
        case crc32("weave"): return 50;
        case crc32("qualification"): return 51;
        case crc32("lifestyle"): return 52;
        case crc32("dairy"): return 53;
        case crc32("empirical"): return 54;
        case crc32("golf"): return 55;
        case crc32("particle"): return 56;
        case crc32("virus"): return 57;
        case crc32("support"): return 58;
        case crc32("noise"): return 59;
        case crc32("old age"): return 60;
        case crc32("spend"): return 61;
        case crc32("punch"): return 62;
        case crc32("deck"): return 63;
        case crc32("commemorate"): return 64;
        case crc32("excess"): return 65;
        case crc32("recognize"): return 66;
        case crc32("hay"): return 67;
        case crc32("speed"): return 68;
        case crc32("salt"): return 69;
        case crc32("constraint"): return 70;
        case crc32("bishop"): return 71;
        case crc32("belief"): return 72;
        case crc32("aids"): return 73;
        case crc32("iron"): return 74;
        case crc32("stop"): return 75;
        case crc32("human body"): return 76;
        case crc32("please"): return 77;
        case crc32("assault"): return 78;
        case crc32("arch"): return 79;
        case crc32("ethics"): return 80;
        case crc32("nap"): return 81;
        case crc32("winner"): return 82;
        case crc32("magnitude"): return 83;
        case crc32("candidate"): return 84;
        case crc32("compromise"): return 85;
        case crc32("enhance"): return 86;
        case crc32("purpose"): return 87;
        case crc32("tradition"): return 88;
        case crc32("album"): return 89;
        case crc32("retire"): return 90;
        case crc32("anticipation"): return 91;
        case crc32("log"): return 92;
        case crc32("talented"): return 93;
        case crc32("refund"): return 94;
        case crc32("resort"): return 95;
        case crc32("observation"): return 96;
        case crc32("lake"): return 97;
        case crc32("siege"): return 98;
        case crc32("balance"): return 99;
        case crc32("tell"): return 100;
        default: __assume(0);
    }
}

inline int SwitchValue (const char* s) {
    return SwitchValueRaw(crc32(s));
}

inline int SwitchValueValidateRaw (const char* s, unsigned int hash) {
    switch(hash) {
        case crc32("nuance"): if (!strcmp(s, "nuance")) return 1; else break;
        case crc32("vertical"): if (!strcmp(s, "vertical")) return 2; else break;
        case crc32("projection"): if (!strcmp(s, "projection")) return 3; else break;
        case crc32("peace"): if (!strcmp(s, "peace")) return 4; else break;
        case crc32("exile"): if (!strcmp(s, "exile")) return 5; else break;
        case crc32("shop"): if (!strcmp(s, "shop")) return 6; else break;
        case crc32("situation"): if (!strcmp(s, "situation")) return 7; else break;
        case crc32("bell"): if (!strcmp(s, "bell")) return 8; else break;
        case crc32("fade"): if (!strcmp(s, "fade")) return 9; else break;
        case crc32("conviction"): if (!strcmp(s, "conviction")) return 10; else break;
        case crc32("cutting"): if (!strcmp(s, "cutting")) return 11; else break;
        case crc32("television"): if (!strcmp(s, "television")) return 12; else break;
        case crc32("shot"): if (!strcmp(s, "shot")) return 13; else break;
        case crc32("momentum"): if (!strcmp(s, "momentum")) return 14; else break;
        case crc32("pride"): if (!strcmp(s, "pride")) return 15; else break;
        case crc32("betray"): if (!strcmp(s, "betray")) return 16; else break;
        case crc32("clock"): if (!strcmp(s, "clock")) return 17; else break;
        case crc32("glide"): if (!strcmp(s, "glide")) return 18; else break;
        case crc32("session"): if (!strcmp(s, "session")) return 19; else break;
        case crc32("explain"): if (!strcmp(s, "explain")) return 20; else break;
        case crc32("abortion"): if (!strcmp(s, "abortion")) return 21; else break;
        case crc32("shake"): if (!strcmp(s, "shake")) return 22; else break;
        case crc32("stunning"): if (!strcmp(s, "stunning")) return 23; else break;
        case crc32("neutral"): if (!strcmp(s, "neutral")) return 24; else break;
        case crc32("productive"): if (!strcmp(s, "productive")) return 25; else break;
        case crc32("daughter"): if (!strcmp(s, "daughter")) return 26; else break;
        case crc32("pig"): if (!strcmp(s, "pig")) return 27; else break;
        case crc32("correspond"): if (!strcmp(s, "correspond")) return 28; else break;
        case crc32("barrier"): if (!strcmp(s, "barrier")) return 29; else break;
        case crc32("perfume"): if (!strcmp(s, "perfume")) return 30; else break;
        case crc32("timber"): if (!strcmp(s, "timber")) return 31; else break;
        case crc32("plaintiff"): if (!strcmp(s, "plaintiff")) return 32; else break;
        case crc32("oven"): if (!strcmp(s, "oven")) return 33; else break;
        case crc32("depend"): if (!strcmp(s, "depend")) return 34; else break;
        case crc32("frozen"): if (!strcmp(s, "frozen")) return 35; else break;
        case crc32("facade"): if (!strcmp(s, "facade")) return 36; else break;
        case crc32("drawer"): if (!strcmp(s, "drawer")) return 37; else break;
        case crc32("maze"): if (!strcmp(s, "maze")) return 38; else break;
        case crc32("information"): if (!strcmp(s, "information")) return 39; else break;
        case crc32("bird"): if (!strcmp(s, "bird")) return 40; else break;
        case crc32("pole"): if (!strcmp(s, "pole")) return 41; else break;
        case crc32("ignite"): if (!strcmp(s, "ignite")) return 42; else break;
        case crc32("suspect"): if (!strcmp(s, "suspect")) return 43; else break;
        case crc32("parameter"): if (!strcmp(s, "parameter")) return 44; else break;
        case crc32("decade"): if (!strcmp(s, "decade")) return 45; else break;
        case crc32("reproduce"): if (!strcmp(s, "reproduce")) return 46; else break;
        case crc32("describe"): if (!strcmp(s, "describe")) return 47; else break;
        case crc32("jealous"): if (!strcmp(s, "jealous")) return 48; else break;
        case crc32("discriminate"): if (!strcmp(s, "discriminate")) return 49; else break;
        case crc32("weave"): if (!strcmp(s, "weave")) return 50; else break;
        case crc32("qualification"): if (!strcmp(s, "qualification")) return 51; else break;
        case crc32("lifestyle"): if (!strcmp(s, "lifestyle")) return 52; else break;
        case crc32("dairy"): if (!strcmp(s, "dairy")) return 53; else break;
        case crc32("empirical"): if (!strcmp(s, "empirical")) return 54; else break;
        case crc32("golf"): if (!strcmp(s, "golf")) return 55; else break;
        case crc32("particle"): if (!strcmp(s, "particle")) return 56; else break;
        case crc32("virus"): if (!strcmp(s, "virus")) return 57; else break;
        case crc32("support"): if (!strcmp(s, "support")) return 58; else break;
        case crc32("noise"): if (!strcmp(s, "noise")) return 59; else break;
        case crc32("old age"): if (!strcmp(s, "old age")) return 60; else break;
        case crc32("spend"): if (!strcmp(s, "spend")) return 61; else break;
        case crc32("punch"): if (!strcmp(s, "punch")) return 62; else break;
        case crc32("deck"): if (!strcmp(s, "deck")) return 63; else break;
        case crc32("commemorate"): if (!strcmp(s, "commemorate")) return 64; else break;
        case crc32("excess"): if (!strcmp(s, "excess")) return 65; else break;
        case crc32("recognize"): if (!strcmp(s, "recognize")) return 66; else break;
        case crc32("hay"): if (!strcmp(s, "hay")) return 67; else break;
        case crc32("speed"): if (!strcmp(s, "speed")) return 68; else break;
        case crc32("salt"): if (!strcmp(s, "salt")) return 69; else break;
        case crc32("constraint"): if (!strcmp(s, "constraint")) return 70; else break;
        case crc32("bishop"): if (!strcmp(s, "bishop")) return 71; else break;
        case crc32("belief"): if (!strcmp(s, "belief")) return 72; else break;
        case crc32("aids"): if (!strcmp(s, "aids")) return 73; else break;
        case crc32("iron"): if (!strcmp(s, "iron")) return 74; else break;
        case crc32("stop"): if (!strcmp(s, "stop")) return 75; else break;
        case crc32("human body"): if (!strcmp(s, "human body")) return 76; else break;
        case crc32("please"): if (!strcmp(s, "please")) return 77; else break;
        case crc32("assault"): if (!strcmp(s, "assault")) return 78; else break;
        case crc32("arch"): if (!strcmp(s, "arch")) return 79; else break;
        case crc32("ethics"): if (!strcmp(s, "ethics")) return 80; else break;
        case crc32("nap"): if (!strcmp(s, "nap")) return 81; else break;
        case crc32("winner"): if (!strcmp(s, "winner")) return 82; else break;
        case crc32("magnitude"): if (!strcmp(s, "magnitude")) return 83; else break;
        case crc32("candidate"): if (!strcmp(s, "candidate")) return 84; else break;
        case crc32("compromise"): if (!strcmp(s, "compromise")) return 85; else break;
        case crc32("enhance"): if (!strcmp(s, "enhance")) return 86; else break;
        case crc32("purpose"): if (!strcmp(s, "purpose")) return 87; else break;
        case crc32("tradition"): if (!strcmp(s, "tradition")) return 88; else break;
        case crc32("album"): if (!strcmp(s, "album")) return 89; else break;
        case crc32("retire"): if (!strcmp(s, "retire")) return 90; else break;
        case crc32("anticipation"): if (!strcmp(s, "anticipation")) return 91; else break;
        case crc32("log"): if (!strcmp(s, "log")) return 92; else break;
        case crc32("talented"): if (!strcmp(s, "talented")) return 93; else break;
        case crc32("refund"): if (!strcmp(s, "refund")) return 94; else break;
        case crc32("resort"): if (!strcmp(s, "resort")) return 95; else break;
        case crc32("observation"): if (!strcmp(s, "observation")) return 96; else break;
        case crc32("lake"): if (!strcmp(s, "lake")) return 97; else break;
        case crc32("siege"): if (!strcmp(s, "siege")) return 98; else break;
        case crc32("balance"): if (!strcmp(s, "balance")) return 99; else break;
        case crc32("tell"): if (!strcmp(s, "tell")) return 100; else break;
    }
    return 0;
}

inline int SwitchValueValidate (const char* s) {
    return SwitchValueValidateRaw(s, crc32(s));
}

inline int SwitchValueMinimizedRaw (unsigned int hash) {
    switch(hash) {
        case (crc32("nuance", c_salt) % c_numHashBuckets): return 1;
        case (crc32("vertical", c_salt) % c_numHashBuckets): return 2;
        case (crc32("projection", c_salt) % c_numHashBuckets): return 3;
        case (crc32("peace", c_salt) % c_numHashBuckets): return 4;
        case (crc32("exile", c_salt) % c_numHashBuckets): return 5;
        case (crc32("shop", c_salt) % c_numHashBuckets): return 6;
        case (crc32("situation", c_salt) % c_numHashBuckets): return 7;
        case (crc32("bell", c_salt) % c_numHashBuckets): return 8;
        case (crc32("fade", c_salt) % c_numHashBuckets): return 9;
        case (crc32("conviction", c_salt) % c_numHashBuckets): return 10;
        case (crc32("cutting", c_salt) % c_numHashBuckets): return 11;
        case (crc32("television", c_salt) % c_numHashBuckets): return 12;
        case (crc32("shot", c_salt) % c_numHashBuckets): return 13;
        case (crc32("momentum", c_salt) % c_numHashBuckets): return 14;
        case (crc32("pride", c_salt) % c_numHashBuckets): return 15;
        case (crc32("betray", c_salt) % c_numHashBuckets): return 16;
        case (crc32("clock", c_salt) % c_numHashBuckets): return 17;
        case (crc32("glide", c_salt) % c_numHashBuckets): return 18;
        case (crc32("session", c_salt) % c_numHashBuckets): return 19;
        case (crc32("explain", c_salt) % c_numHashBuckets): return 20;
        case (crc32("abortion", c_salt) % c_numHashBuckets): return 21;
        case (crc32("shake", c_salt) % c_numHashBuckets): return 22;
        case (crc32("stunning", c_salt) % c_numHashBuckets): return 23;
        case (crc32("neutral", c_salt) % c_numHashBuckets): return 24;
        case (crc32("productive", c_salt) % c_numHashBuckets): return 25;
        case (crc32("daughter", c_salt) % c_numHashBuckets): return 26;
        case (crc32("pig", c_salt) % c_numHashBuckets): return 27;
        case (crc32("correspond", c_salt) % c_numHashBuckets): return 28;
        case (crc32("barrier", c_salt) % c_numHashBuckets): return 29;
        case (crc32("perfume", c_salt) % c_numHashBuckets): return 30;
        case (crc32("timber", c_salt) % c_numHashBuckets): return 31;
        case (crc32("plaintiff", c_salt) % c_numHashBuckets): return 32;
        case (crc32("oven", c_salt) % c_numHashBuckets): return 33;
        case (crc32("depend", c_salt) % c_numHashBuckets): return 34;
        case (crc32("frozen", c_salt) % c_numHashBuckets): return 35;
        case (crc32("facade", c_salt) % c_numHashBuckets): return 36;
        case (crc32("drawer", c_salt) % c_numHashBuckets): return 37;
        case (crc32("maze", c_salt) % c_numHashBuckets): return 38;
        case (crc32("information", c_salt) % c_numHashBuckets): return 39;
        case (crc32("bird", c_salt) % c_numHashBuckets): return 40;
        case (crc32("pole", c_salt) % c_numHashBuckets): return 41;
        case (crc32("ignite", c_salt) % c_numHashBuckets): return 42;
        case (crc32("suspect", c_salt) % c_numHashBuckets): return 43;
        case (crc32("parameter", c_salt) % c_numHashBuckets): return 44;
        case (crc32("decade", c_salt) % c_numHashBuckets): return 45;
        case (crc32("reproduce", c_salt) % c_numHashBuckets): return 46;
        case (crc32("describe", c_salt) % c_numHashBuckets): return 47;
        case (crc32("jealous", c_salt) % c_numHashBuckets): return 48;
        case (crc32("discriminate", c_salt) % c_numHashBuckets): return 49;
        case (crc32("weave", c_salt) % c_numHashBuckets): return 50;
        case (crc32("qualification", c_salt) % c_numHashBuckets): return 51;
        case (crc32("lifestyle", c_salt) % c_numHashBuckets): return 52;
        case (crc32("dairy", c_salt) % c_numHashBuckets): return 53;
        case (crc32("empirical", c_salt) % c_numHashBuckets): return 54;
        case (crc32("golf", c_salt) % c_numHashBuckets): return 55;
        case (crc32("particle", c_salt) % c_numHashBuckets): return 56;
        case (crc32("virus", c_salt) % c_numHashBuckets): return 57;
        case (crc32("support", c_salt) % c_numHashBuckets): return 58;
        case (crc32("noise", c_salt) % c_numHashBuckets): return 59;
        case (crc32("old age", c_salt) % c_numHashBuckets): return 60;
        case (crc32("spend", c_salt) % c_numHashBuckets): return 61;
        case (crc32("punch", c_salt) % c_numHashBuckets): return 62;
        case (crc32("deck", c_salt) % c_numHashBuckets): return 63;
        case (crc32("commemorate", c_salt) % c_numHashBuckets): return 64;
        case (crc32("excess", c_salt) % c_numHashBuckets): return 65;
        case (crc32("recognize", c_salt) % c_numHashBuckets): return 66;
        case (crc32("hay", c_salt) % c_numHashBuckets): return 67;
        case (crc32("speed", c_salt) % c_numHashBuckets): return 68;
        case (crc32("salt", c_salt) % c_numHashBuckets): return 69;
        case (crc32("constraint", c_salt) % c_numHashBuckets): return 70;
        case (crc32("bishop", c_salt) % c_numHashBuckets): return 71;
        case (crc32("belief", c_salt) % c_numHashBuckets): return 72;
        case (crc32("aids", c_salt) % c_numHashBuckets): return 73;
        case (crc32("iron", c_salt) % c_numHashBuckets): return 74;
        case (crc32("stop", c_salt) % c_numHashBuckets): return 75;
        case (crc32("human body", c_salt) % c_numHashBuckets): return 76;
        case (crc32("please", c_salt) % c_numHashBuckets): return 77;
        case (crc32("assault", c_salt) % c_numHashBuckets): return 78;
        case (crc32("arch", c_salt) % c_numHashBuckets): return 79;
        case (crc32("ethics", c_salt) % c_numHashBuckets): return 80;
        case (crc32("nap", c_salt) % c_numHashBuckets): return 81;
        case (crc32("winner", c_salt) % c_numHashBuckets): return 82;
        case (crc32("magnitude", c_salt) % c_numHashBuckets): return 83;
        case (crc32("candidate", c_salt) % c_numHashBuckets): return 84;
        case (crc32("compromise", c_salt) % c_numHashBuckets): return 85;
        case (crc32("enhance", c_salt) % c_numHashBuckets): return 86;
        case (crc32("purpose", c_salt) % c_numHashBuckets): return 87;
        case (crc32("tradition", c_salt) % c_numHashBuckets): return 88;
        case (crc32("album", c_salt) % c_numHashBuckets): return 89;
        case (crc32("retire", c_salt) % c_numHashBuckets): return 90;
        case (crc32("anticipation", c_salt) % c_numHashBuckets): return 91;
        case (crc32("log", c_salt) % c_numHashBuckets): return 92;
        case (crc32("talented", c_salt) % c_numHashBuckets): return 93;
        case (crc32("refund", c_salt) % c_numHashBuckets): return 94;
        case (crc32("resort", c_salt) % c_numHashBuckets): return 95;
        case (crc32("observation", c_salt) % c_numHashBuckets): return 96;
        case (crc32("lake", c_salt) % c_numHashBuckets): return 97;
        case (crc32("siege", c_salt) % c_numHashBuckets): return 98;
        case (crc32("balance", c_salt) % c_numHashBuckets): return 99;
        case (crc32("tell", c_salt) % c_numHashBuckets): return 100;
        default: __assume(0);
    }
}

inline int SwitchValueMinimized (const char* s) {
    return SwitchValueMinimizedRaw(crc32(s, c_salt) % c_numHashBuckets);
}

inline int SwitchValueMinimizedValidateRaw (const char* s, unsigned int hash) {
    switch(hash) {
        case (crc32("nuance", c_salt) % c_numHashBuckets): if (!strcmp(s, "nuance")) return 1; else break;
        case (crc32("vertical", c_salt) % c_numHashBuckets): if (!strcmp(s, "vertical")) return 2; else break;
        case (crc32("projection", c_salt) % c_numHashBuckets): if (!strcmp(s, "projection")) return 3; else break;
        case (crc32("peace", c_salt) % c_numHashBuckets): if (!strcmp(s, "peace")) return 4; else break;
        case (crc32("exile", c_salt) % c_numHashBuckets): if (!strcmp(s, "exile")) return 5; else break;
        case (crc32("shop", c_salt) % c_numHashBuckets): if (!strcmp(s, "shop")) return 6; else break;
        case (crc32("situation", c_salt) % c_numHashBuckets): if (!strcmp(s, "situation")) return 7; else break;
        case (crc32("bell", c_salt) % c_numHashBuckets): if (!strcmp(s, "bell")) return 8; else break;
        case (crc32("fade", c_salt) % c_numHashBuckets): if (!strcmp(s, "fade")) return 9; else break;
        case (crc32("conviction", c_salt) % c_numHashBuckets): if (!strcmp(s, "conviction")) return 10; else break;
        case (crc32("cutting", c_salt) % c_numHashBuckets): if (!strcmp(s, "cutting")) return 11; else break;
        case (crc32("television", c_salt) % c_numHashBuckets): if (!strcmp(s, "television")) return 12; else break;
        case (crc32("shot", c_salt) % c_numHashBuckets): if (!strcmp(s, "shot")) return 13; else break;
        case (crc32("momentum", c_salt) % c_numHashBuckets): if (!strcmp(s, "momentum")) return 14; else break;
        case (crc32("pride", c_salt) % c_numHashBuckets): if (!strcmp(s, "pride")) return 15; else break;
        case (crc32("betray", c_salt) % c_numHashBuckets): if (!strcmp(s, "betray")) return 16; else break;
        case (crc32("clock", c_salt) % c_numHashBuckets): if (!strcmp(s, "clock")) return 17; else break;
        case (crc32("glide", c_salt) % c_numHashBuckets): if (!strcmp(s, "glide")) return 18; else break;
        case (crc32("session", c_salt) % c_numHashBuckets): if (!strcmp(s, "session")) return 19; else break;
        case (crc32("explain", c_salt) % c_numHashBuckets): if (!strcmp(s, "explain")) return 20; else break;
        case (crc32("abortion", c_salt) % c_numHashBuckets): if (!strcmp(s, "abortion")) return 21; else break;
        case (crc32("shake", c_salt) % c_numHashBuckets): if (!strcmp(s, "shake")) return 22; else break;
        case (crc32("stunning", c_salt) % c_numHashBuckets): if (!strcmp(s, "stunning")) return 23; else break;
        case (crc32("neutral", c_salt) % c_numHashBuckets): if (!strcmp(s, "neutral")) return 24; else break;
        case (crc32("productive", c_salt) % c_numHashBuckets): if (!strcmp(s, "productive")) return 25; else break;
        case (crc32("daughter", c_salt) % c_numHashBuckets): if (!strcmp(s, "daughter")) return 26; else break;
        case (crc32("pig", c_salt) % c_numHashBuckets): if (!strcmp(s, "pig")) return 27; else break;
        case (crc32("correspond", c_salt) % c_numHashBuckets): if (!strcmp(s, "correspond")) return 28; else break;
        case (crc32("barrier", c_salt) % c_numHashBuckets): if (!strcmp(s, "barrier")) return 29; else break;
        case (crc32("perfume", c_salt) % c_numHashBuckets): if (!strcmp(s, "perfume")) return 30; else break;
        case (crc32("timber", c_salt) % c_numHashBuckets): if (!strcmp(s, "timber")) return 31; else break;
        case (crc32("plaintiff", c_salt) % c_numHashBuckets): if (!strcmp(s, "plaintiff")) return 32; else break;
        case (crc32("oven", c_salt) % c_numHashBuckets): if (!strcmp(s, "oven")) return 33; else break;
        case (crc32("depend", c_salt) % c_numHashBuckets): if (!strcmp(s, "depend")) return 34; else break;
        case (crc32("frozen", c_salt) % c_numHashBuckets): if (!strcmp(s, "frozen")) return 35; else break;
        case (crc32("facade", c_salt) % c_numHashBuckets): if (!strcmp(s, "facade")) return 36; else break;
        case (crc32("drawer", c_salt) % c_numHashBuckets): if (!strcmp(s, "drawer")) return 37; else break;
        case (crc32("maze", c_salt) % c_numHashBuckets): if (!strcmp(s, "maze")) return 38; else break;
        case (crc32("information", c_salt) % c_numHashBuckets): if (!strcmp(s, "information")) return 39; else break;
        case (crc32("bird", c_salt) % c_numHashBuckets): if (!strcmp(s, "bird")) return 40; else break;
        case (crc32("pole", c_salt) % c_numHashBuckets): if (!strcmp(s, "pole")) return 41; else break;
        case (crc32("ignite", c_salt) % c_numHashBuckets): if (!strcmp(s, "ignite")) return 42; else break;
        case (crc32("suspect", c_salt) % c_numHashBuckets): if (!strcmp(s, "suspect")) return 43; else break;
        case (crc32("parameter", c_salt) % c_numHashBuckets): if (!strcmp(s, "parameter")) return 44; else break;
        case (crc32("decade", c_salt) % c_numHashBuckets): if (!strcmp(s, "decade")) return 45; else break;
        case (crc32("reproduce", c_salt) % c_numHashBuckets): if (!strcmp(s, "reproduce")) return 46; else break;
        case (crc32("describe", c_salt) % c_numHashBuckets): if (!strcmp(s, "describe")) return 47; else break;
        case (crc32("jealous", c_salt) % c_numHashBuckets): if (!strcmp(s, "jealous")) return 48; else break;
        case (crc32("discriminate", c_salt) % c_numHashBuckets): if (!strcmp(s, "discriminate")) return 49; else break;
        case (crc32("weave", c_salt) % c_numHashBuckets): if (!strcmp(s, "weave")) return 50; else break;
        case (crc32("qualification", c_salt) % c_numHashBuckets): if (!strcmp(s, "qualification")) return 51; else break;
        case (crc32("lifestyle", c_salt) % c_numHashBuckets): if (!strcmp(s, "lifestyle")) return 52; else break;
        case (crc32("dairy", c_salt) % c_numHashBuckets): if (!strcmp(s, "dairy")) return 53; else break;
        case (crc32("empirical", c_salt) % c_numHashBuckets): if (!strcmp(s, "empirical")) return 54; else break;
        case (crc32("golf", c_salt) % c_numHashBuckets): if (!strcmp(s, "golf")) return 55; else break;
        case (crc32("particle", c_salt) % c_numHashBuckets): if (!strcmp(s, "particle")) return 56; else break;
        case (crc32("virus", c_salt) % c_numHashBuckets): if (!strcmp(s, "virus")) return 57; else break;
        case (crc32("support", c_salt) % c_numHashBuckets): if (!strcmp(s, "support")) return 58; else break;
        case (crc32("noise", c_salt) % c_numHashBuckets): if (!strcmp(s, "noise")) return 59; else break;
        case (crc32("old age", c_salt) % c_numHashBuckets): if (!strcmp(s, "old age")) return 60; else break;
        case (crc32("spend", c_salt) % c_numHashBuckets): if (!strcmp(s, "spend")) return 61; else break;
        case (crc32("punch", c_salt) % c_numHashBuckets): if (!strcmp(s, "punch")) return 62; else break;
        case (crc32("deck", c_salt) % c_numHashBuckets): if (!strcmp(s, "deck")) return 63; else break;
        case (crc32("commemorate", c_salt) % c_numHashBuckets): if (!strcmp(s, "commemorate")) return 64; else break;
        case (crc32("excess", c_salt) % c_numHashBuckets): if (!strcmp(s, "excess")) return 65; else break;
        case (crc32("recognize", c_salt) % c_numHashBuckets): if (!strcmp(s, "recognize")) return 66; else break;
        case (crc32("hay", c_salt) % c_numHashBuckets): if (!strcmp(s, "hay")) return 67; else break;
        case (crc32("speed", c_salt) % c_numHashBuckets): if (!strcmp(s, "speed")) return 68; else break;
        case (crc32("salt", c_salt) % c_numHashBuckets): if (!strcmp(s, "salt")) return 69; else break;
        case (crc32("constraint", c_salt) % c_numHashBuckets): if (!strcmp(s, "constraint")) return 70; else break;
        case (crc32("bishop", c_salt) % c_numHashBuckets): if (!strcmp(s, "bishop")) return 71; else break;
        case (crc32("belief", c_salt) % c_numHashBuckets): if (!strcmp(s, "belief")) return 72; else break;
        case (crc32("aids", c_salt) % c_numHashBuckets): if (!strcmp(s, "aids")) return 73; else break;
        case (crc32("iron", c_salt) % c_numHashBuckets): if (!strcmp(s, "iron")) return 74; else break;
        case (crc32("stop", c_salt) % c_numHashBuckets): if (!strcmp(s, "stop")) return 75; else break;
        case (crc32("human body", c_salt) % c_numHashBuckets): if (!strcmp(s, "human body")) return 76; else break;
        case (crc32("please", c_salt) % c_numHashBuckets): if (!strcmp(s, "please")) return 77; else break;
        case (crc32("assault", c_salt) % c_numHashBuckets): if (!strcmp(s, "assault")) return 78; else break;
        case (crc32("arch", c_salt) % c_numHashBuckets): if (!strcmp(s, "arch")) return 79; else break;
        case (crc32("ethics", c_salt) % c_numHashBuckets): if (!strcmp(s, "ethics")) return 80; else break;
        case (crc32("nap", c_salt) % c_numHashBuckets): if (!strcmp(s, "nap")) return 81; else break;
        case (crc32("winner", c_salt) % c_numHashBuckets): if (!strcmp(s, "winner")) return 82; else break;
        case (crc32("magnitude", c_salt) % c_numHashBuckets): if (!strcmp(s, "magnitude")) return 83; else break;
        case (crc32("candidate", c_salt) % c_numHashBuckets): if (!strcmp(s, "candidate")) return 84; else break;
        case (crc32("compromise", c_salt) % c_numHashBuckets): if (!strcmp(s, "compromise")) return 85; else break;
        case (crc32("enhance", c_salt) % c_numHashBuckets): if (!strcmp(s, "enhance")) return 86; else break;
        case (crc32("purpose", c_salt) % c_numHashBuckets): if (!strcmp(s, "purpose")) return 87; else break;
        case (crc32("tradition", c_salt) % c_numHashBuckets): if (!strcmp(s, "tradition")) return 88; else break;
        case (crc32("album", c_salt) % c_numHashBuckets): if (!strcmp(s, "album")) return 89; else break;
        case (crc32("retire", c_salt) % c_numHashBuckets): if (!strcmp(s, "retire")) return 90; else break;
        case (crc32("anticipation", c_salt) % c_numHashBuckets): if (!strcmp(s, "anticipation")) return 91; else break;
        case (crc32("log", c_salt) % c_numHashBuckets): if (!strcmp(s, "log")) return 92; else break;
        case (crc32("talented", c_salt) % c_numHashBuckets): if (!strcmp(s, "talented")) return 93; else break;
        case (crc32("refund", c_salt) % c_numHashBuckets): if (!strcmp(s, "refund")) return 94; else break;
        case (crc32("resort", c_salt) % c_numHashBuckets): if (!strcmp(s, "resort")) return 95; else break;
        case (crc32("observation", c_salt) % c_numHashBuckets): if (!strcmp(s, "observation")) return 96; else break;
        case (crc32("lake", c_salt) % c_numHashBuckets): if (!strcmp(s, "lake")) return 97; else break;
        case (crc32("siege", c_salt) % c_numHashBuckets): if (!strcmp(s, "siege")) return 98; else break;
        case (crc32("balance", c_salt) % c_numHashBuckets): if (!strcmp(s, "balance")) return 99; else break;
        case (crc32("tell", c_salt) % c_numHashBuckets): if (!strcmp(s, "tell")) return 100; else break;
    }
    return 0;
}

inline int SwitchValueMinimizedValidate (const char* s) {
    return SwitchValueMinimizedValidateRaw(s, crc32(s, c_salt) % c_numHashBuckets);
}

int g_SwitchValueMinimizedArray[c_numHashBuckets] = {
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    91, 
    0, // Unused bucket, wasted memory
    29, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    39, 
    72, 
    84, 
    0, // Unused bucket, wasted memory
    73, 
    0, // Unused bucket, wasted memory
    96, 
    13, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    44, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    37, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    80, 
    0, // Unused bucket, wasted memory
    3, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    98, 
    20, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    87, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    19, 
    59, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    95, 
    83, 
    12, 
    10, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    55, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    63, 
    36, 
    50, 
    33, 
    0, // Unused bucket, wasted memory
    53, 
    0, // Unused bucket, wasted memory
    60, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    74, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    100, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    6, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    61, 
    42, 
    0, // Unused bucket, wasted memory
    15, 
    0, // Unused bucket, wasted memory
    11, 
    0, // Unused bucket, wasted memory
    62, 
    0, // Unused bucket, wasted memory
    2, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    86, 
    57, 
    47, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    31, 
    0, // Unused bucket, wasted memory
    21, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    35, 
    0, // Unused bucket, wasted memory
    40, 
    0, // Unused bucket, wasted memory
    94, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    1, 
    17, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    27, 
    75, 
    88, 
    58, 
    0, // Unused bucket, wasted memory
    99, 
    32, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    66, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    25, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    78, 
    22, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    24, 
    23, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    89, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    64, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    52, 
    14, 
    0, // Unused bucket, wasted memory
    16, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    81, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    92, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    34, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    97, 
    71, 
    0, // Unused bucket, wasted memory
    41, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    49, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    70, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    48, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    85, 
    0, // Unused bucket, wasted memory
    28, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    43, 
    93, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    56, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    18, 
    68, 
    8, 
    77, 
    0, // Unused bucket, wasted memory
    69, 
    45, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    30, 
    54, 
    0, // Unused bucket, wasted memory
    51, 
    0, // Unused bucket, wasted memory
    82, 
    0, // Unused bucket, wasted memory
    9, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    7, 
    0, // Unused bucket, wasted memory
    4, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    76, 
    0, // Unused bucket, wasted memory
    67, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    26, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    46, 
    79, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    5, 
    38, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    90, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    65, 
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
    0, // Unused bucket, wasted memory
};

struct SValidate {
    const char* m_string;
    int         m_value;
};

SValidate g_SwitchValueMinimizedArrayValidate[c_numHashBuckets] = {
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"anticipation", 91}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"barrier", 29}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"information", 39}, 
    {"belief", 72}, 
    {"candidate", 84}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"aids", 73}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"observation", 96}, 
    {"shot", 13}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"parameter", 44}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"drawer", 37}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"ethics", 80}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"projection", 3}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"siege", 98}, 
    {"explain", 20}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"purpose", 87}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"session", 19}, 
    {"noise", 59}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"resort", 95}, 
    {"magnitude", 83}, 
    {"television", 12}, 
    {"conviction", 10}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"golf", 55}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"deck", 63}, 
    {"facade", 36}, 
    {"weave", 50}, 
    {"oven", 33}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"dairy", 53}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"old age", 60}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"iron", 74}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"tell", 100}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"shop", 6}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"spend", 61}, 
    {"ignite", 42}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"pride", 15}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"cutting", 11}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"punch", 62}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"vertical", 2}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"enhance", 86}, 
    {"virus", 57}, 
    {"describe", 47}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"timber", 31}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"abortion", 21}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"frozen", 35}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"bird", 40}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"refund", 94}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"nuance", 1}, 
    {"clock", 17}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"pig", 27}, 
    {"stop", 75}, 
    {"tradition", 88}, 
    {"support", 58}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"balance", 99}, 
    {"plaintiff", 32}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"recognize", 66}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"productive", 25}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"assault", 78}, 
    {"shake", 22}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"neutral", 24}, 
    {"stunning", 23}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"album", 89}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"commemorate", 64}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"lifestyle", 52}, 
    {"momentum", 14}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"betray", 16}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"nap", 81}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"log", 92}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"depend", 34}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"lake", 97}, 
    {"bishop", 71}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"pole", 41}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"discriminate", 49}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"constraint", 70}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"jealous", 48}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"compromise", 85}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"correspond", 28}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"suspect", 43}, 
    {"talented", 93}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"particle", 56}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"glide", 18}, 
    {"speed", 68}, 
    {"bell", 8}, 
    {"please", 77}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"salt", 69}, 
    {"decade", 45}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"perfume", 30}, 
    {"empirical", 54}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"qualification", 51}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"winner", 82}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"fade", 9}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"situation", 7}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"peace", 4}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"human body", 76}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {"hay", 67}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"daughter", 26}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"reproduce", 46}, 
    {"arch", 79}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"exile", 5}, 
    {"maze", 38}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"retire", 90}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {"excess", 65}, 
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
    {nullptr, 0}, // Unused bucket, wasted memory
};

int BruteForceByStartingLetter (const char* s) {
    switch(*s) {
        case 'a': {
            if(!strcmp(&s[1], "bortion"))
                return 20;
            else if(!strcmp(&s[1], "ids"))
                return 72;
            else if(!strcmp(&s[1], "ssault"))
                return 77;
            else if(!strcmp(&s[1], "rch"))
                return 78;
            else if(!strcmp(&s[1], "lbum"))
                return 88;
            else if(!strcmp(&s[1], "nticipation"))
                return 90;
            break;
        }
        case 'b': {
            if(!strcmp(&s[1], "ell"))
                return 7;
            else if(!strcmp(&s[1], "etray"))
                return 15;
            else if(!strcmp(&s[1], "arrier"))
                return 28;
            else if(!strcmp(&s[1], "ird"))
                return 39;
            else if(!strcmp(&s[1], "ishop"))
                return 70;
            else if(!strcmp(&s[1], "elief"))
                return 71;
            else if(!strcmp(&s[1], "alance"))
                return 98;
            break;
        }
        case 'c': {
            if(!strcmp(&s[1], "onviction"))
                return 9;
            else if(!strcmp(&s[1], "utting"))
                return 10;
            else if(!strcmp(&s[1], "lock"))
                return 16;
            else if(!strcmp(&s[1], "orrespond"))
                return 27;
            else if(!strcmp(&s[1], "ommemorate"))
                return 63;
            else if(!strcmp(&s[1], "onstraint"))
                return 69;
            else if(!strcmp(&s[1], "andidate"))
                return 83;
            else if(!strcmp(&s[1], "ompromise"))
                return 84;
            break;
        }
        case 'd': {
            if(!strcmp(&s[1], "aughter"))
                return 25;
            else if(!strcmp(&s[1], "epend"))
                return 33;
            else if(!strcmp(&s[1], "rawer"))
                return 36;
            else if(!strcmp(&s[1], "ecade"))
                return 44;
            else if(!strcmp(&s[1], "escribe"))
                return 46;
            else if(!strcmp(&s[1], "iscriminate"))
                return 48;
            else if(!strcmp(&s[1], "airy"))
                return 52;
            else if(!strcmp(&s[1], "eck"))
                return 62;
            break;
        }
        case 'e': {
            if(!strcmp(&s[1], "xile"))
                return 4;
            else if(!strcmp(&s[1], "xplain"))
                return 19;
            else if(!strcmp(&s[1], "mpirical"))
                return 53;
            else if(!strcmp(&s[1], "xcess"))
                return 64;
            else if(!strcmp(&s[1], "thics"))
                return 79;
            else if(!strcmp(&s[1], "nhance"))
                return 85;
            break;
        }
        case 'f': {
            if(!strcmp(&s[1], "ade"))
                return 8;
            else if(!strcmp(&s[1], "rozen"))
                return 34;
            else if(!strcmp(&s[1], "acade"))
                return 35;
            break;
        }
        case 'g': {
            if(!strcmp(&s[1], "lide"))
                return 17;
            else if(!strcmp(&s[1], "olf"))
                return 54;
            break;
        }
        case 'h': {
            if(!strcmp(&s[1], "ay"))
                return 66;
            else if(!strcmp(&s[1], "uman body"))
                return 75;
            break;
        }
        case 'i': {
            if(!strcmp(&s[1], "nformation"))
                return 38;
            else if(!strcmp(&s[1], "gnite"))
                return 41;
            else if(!strcmp(&s[1], "ron"))
                return 73;
            break;
        }
        case 'j': {
            if(!strcmp(&s[1], "ealous"))
                return 47;
            break;
        }
        case 'k': {
            break;
        }
        case 'l': {
            if(!strcmp(&s[1], "ifestyle"))
                return 51;
            else if(!strcmp(&s[1], "og"))
                return 91;
            else if(!strcmp(&s[1], "ake"))
                return 96;
            break;
        }
        case 'm': {
            if(!strcmp(&s[1], "omentum"))
                return 13;
            else if(!strcmp(&s[1], "aze"))
                return 37;
            else if(!strcmp(&s[1], "agnitude"))
                return 82;
            break;
        }
        case 'n': {
            if(!strcmp(&s[1], "uance"))
                return 0;
            else if(!strcmp(&s[1], "eutral"))
                return 23;
            else if(!strcmp(&s[1], "oise"))
                return 58;
            else if(!strcmp(&s[1], "ap"))
                return 80;
            break;
        }
        case 'o': {
            if(!strcmp(&s[1], "ven"))
                return 32;
            else if(!strcmp(&s[1], "ld age"))
                return 59;
            else if(!strcmp(&s[1], "bservation"))
                return 95;
            break;
        }
        case 'p': {
            if(!strcmp(&s[1], "rojection"))
                return 2;
            else if(!strcmp(&s[1], "eace"))
                return 3;
            else if(!strcmp(&s[1], "ride"))
                return 14;
            else if(!strcmp(&s[1], "roductive"))
                return 24;
            else if(!strcmp(&s[1], "ig"))
                return 26;
            else if(!strcmp(&s[1], "erfume"))
                return 29;
            else if(!strcmp(&s[1], "laintiff"))
                return 31;
            else if(!strcmp(&s[1], "ole"))
                return 40;
            else if(!strcmp(&s[1], "arameter"))
                return 43;
            else if(!strcmp(&s[1], "article"))
                return 55;
            else if(!strcmp(&s[1], "unch"))
                return 61;
            else if(!strcmp(&s[1], "lease"))
                return 76;
            else if(!strcmp(&s[1], "urpose"))
                return 86;
            break;
        }
        case 'q': {
            if(!strcmp(&s[1], "ualification"))
                return 50;
            break;
        }
        case 'r': {
            if(!strcmp(&s[1], "eproduce"))
                return 45;
            else if(!strcmp(&s[1], "ecognize"))
                return 65;
            else if(!strcmp(&s[1], "etire"))
                return 89;
            else if(!strcmp(&s[1], "efund"))
                return 93;
            else if(!strcmp(&s[1], "esort"))
                return 94;
            break;
        }
        case 's': {
            if(!strcmp(&s[1], "hop"))
                return 5;
            else if(!strcmp(&s[1], "ituation"))
                return 6;
            else if(!strcmp(&s[1], "hot"))
                return 12;
            else if(!strcmp(&s[1], "ession"))
                return 18;
            else if(!strcmp(&s[1], "hake"))
                return 21;
            else if(!strcmp(&s[1], "tunning"))
                return 22;
            else if(!strcmp(&s[1], "uspect"))
                return 42;
            else if(!strcmp(&s[1], "upport"))
                return 57;
            else if(!strcmp(&s[1], "pend"))
                return 60;
            else if(!strcmp(&s[1], "peed"))
                return 67;
            else if(!strcmp(&s[1], "alt"))
                return 68;
            else if(!strcmp(&s[1], "top"))
                return 74;
            else if(!strcmp(&s[1], "iege"))
                return 97;
            break;
        }
        case 't': {
            if(!strcmp(&s[1], "elevision"))
                return 11;
            else if(!strcmp(&s[1], "imber"))
                return 30;
            else if(!strcmp(&s[1], "radition"))
                return 87;
            else if(!strcmp(&s[1], "alented"))
                return 92;
            else if(!strcmp(&s[1], "ell"))
                return 99;
            break;
        }
        case 'u': {
            break;
        }
        case 'v': {
            if(!strcmp(&s[1], "ertical"))
                return 1;
            else if(!strcmp(&s[1], "irus"))
                return 56;
            break;
        }
        case 'w': {
            if(!strcmp(&s[1], "eave"))
                return 49;
            else if(!strcmp(&s[1], "inner"))
                return 81;
            break;
        }
        case 'x': {
            break;
        }
        case 'y': {
            break;
        }
        case 'z': {
            break;
        }
        default: Fail(); return -1;
    }
    Fail(); return -1;}

int BruteForce (const char* s) {
    if (!strcmp(s, "nuance")) return 1;
    else if (!strcmp(s, "vertical")) return 2;
    else if (!strcmp(s, "projection")) return 3;
    else if (!strcmp(s, "peace")) return 4;
    else if (!strcmp(s, "exile")) return 5;
    else if (!strcmp(s, "shop")) return 6;
    else if (!strcmp(s, "situation")) return 7;
    else if (!strcmp(s, "bell")) return 8;
    else if (!strcmp(s, "fade")) return 9;
    else if (!strcmp(s, "conviction")) return 10;
    else if (!strcmp(s, "cutting")) return 11;
    else if (!strcmp(s, "television")) return 12;
    else if (!strcmp(s, "shot")) return 13;
    else if (!strcmp(s, "momentum")) return 14;
    else if (!strcmp(s, "pride")) return 15;
    else if (!strcmp(s, "betray")) return 16;
    else if (!strcmp(s, "clock")) return 17;
    else if (!strcmp(s, "glide")) return 18;
    else if (!strcmp(s, "session")) return 19;
    else if (!strcmp(s, "explain")) return 20;
    else if (!strcmp(s, "abortion")) return 21;
    else if (!strcmp(s, "shake")) return 22;
    else if (!strcmp(s, "stunning")) return 23;
    else if (!strcmp(s, "neutral")) return 24;
    else if (!strcmp(s, "productive")) return 25;
    else if (!strcmp(s, "daughter")) return 26;
    else if (!strcmp(s, "pig")) return 27;
    else if (!strcmp(s, "correspond")) return 28;
    else if (!strcmp(s, "barrier")) return 29;
    else if (!strcmp(s, "perfume")) return 30;
    else if (!strcmp(s, "timber")) return 31;
    else if (!strcmp(s, "plaintiff")) return 32;
    else if (!strcmp(s, "oven")) return 33;
    else if (!strcmp(s, "depend")) return 34;
    else if (!strcmp(s, "frozen")) return 35;
    else if (!strcmp(s, "facade")) return 36;
    else if (!strcmp(s, "drawer")) return 37;
    else if (!strcmp(s, "maze")) return 38;
    else if (!strcmp(s, "information")) return 39;
    else if (!strcmp(s, "bird")) return 40;
    else if (!strcmp(s, "pole")) return 41;
    else if (!strcmp(s, "ignite")) return 42;
    else if (!strcmp(s, "suspect")) return 43;
    else if (!strcmp(s, "parameter")) return 44;
    else if (!strcmp(s, "decade")) return 45;
    else if (!strcmp(s, "reproduce")) return 46;
    else if (!strcmp(s, "describe")) return 47;
    else if (!strcmp(s, "jealous")) return 48;
    else if (!strcmp(s, "discriminate")) return 49;
    else if (!strcmp(s, "weave")) return 50;
    else if (!strcmp(s, "qualification")) return 51;
    else if (!strcmp(s, "lifestyle")) return 52;
    else if (!strcmp(s, "dairy")) return 53;
    else if (!strcmp(s, "empirical")) return 54;
    else if (!strcmp(s, "golf")) return 55;
    else if (!strcmp(s, "particle")) return 56;
    else if (!strcmp(s, "virus")) return 57;
    else if (!strcmp(s, "support")) return 58;
    else if (!strcmp(s, "noise")) return 59;
    else if (!strcmp(s, "old age")) return 60;
    else if (!strcmp(s, "spend")) return 61;
    else if (!strcmp(s, "punch")) return 62;
    else if (!strcmp(s, "deck")) return 63;
    else if (!strcmp(s, "commemorate")) return 64;
    else if (!strcmp(s, "excess")) return 65;
    else if (!strcmp(s, "recognize")) return 66;
    else if (!strcmp(s, "hay")) return 67;
    else if (!strcmp(s, "speed")) return 68;
    else if (!strcmp(s, "salt")) return 69;
    else if (!strcmp(s, "constraint")) return 70;
    else if (!strcmp(s, "bishop")) return 71;
    else if (!strcmp(s, "belief")) return 72;
    else if (!strcmp(s, "aids")) return 73;
    else if (!strcmp(s, "iron")) return 74;
    else if (!strcmp(s, "stop")) return 75;
    else if (!strcmp(s, "human body")) return 76;
    else if (!strcmp(s, "please")) return 77;
    else if (!strcmp(s, "assault")) return 78;
    else if (!strcmp(s, "arch")) return 79;
    else if (!strcmp(s, "ethics")) return 80;
    else if (!strcmp(s, "nap")) return 81;
    else if (!strcmp(s, "winner")) return 82;
    else if (!strcmp(s, "magnitude")) return 83;
    else if (!strcmp(s, "candidate")) return 84;
    else if (!strcmp(s, "compromise")) return 85;
    else if (!strcmp(s, "enhance")) return 86;
    else if (!strcmp(s, "purpose")) return 87;
    else if (!strcmp(s, "tradition")) return 88;
    else if (!strcmp(s, "album")) return 89;
    else if (!strcmp(s, "retire")) return 90;
    else if (!strcmp(s, "anticipation")) return 91;
    else if (!strcmp(s, "log")) return 92;
    else if (!strcmp(s, "talented")) return 93;
    else if (!strcmp(s, "refund")) return 94;
    else if (!strcmp(s, "resort")) return 95;
    else if (!strcmp(s, "observation")) return 96;
    else if (!strcmp(s, "lake")) return 97;
    else if (!strcmp(s, "siege")) return 98;
    else if (!strcmp(s, "balance")) return 99;
    else if (!strcmp(s, "tell")) return 100;
    Fail(); return -1;
};

