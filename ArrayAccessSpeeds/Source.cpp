#include <stdio.h>
#include <array>
#include <vector>
#include <random>
#include <stdint.h>
#include <limits>
#include <chrono>

#include "Tests.h"

struct SBlockTimer
{
    SBlockTimer(const char* label)
        :m_label(label)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~SBlockTimer()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        printf("%s: %0.2f ms\n", m_label, seconds.count()*1000.0f);
    }

    std::chrono::system_clock::time_point m_start;
    const char* m_label;
};

typedef uint32_t TTestType;
#define TTestTypeName "uint32_t"
#define TTestTypePrintf "%u"

TTestType Random()
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static const TTestType min = std::numeric_limits<TTestType>::min();
    static const TTestType max = std::numeric_limits<TTestType>::max();
    static std::uniform_int_distribution<TTestType> dist(min, max);
    return dist(mt);
}

void GenerateCode (size_t numValues)
{

    FILE *file = fopen("Tests.h", "w+t");
    if (!file)
        return;

    // generate the numbers
    std::vector<TTestType> randomValues;
    randomValues.resize(numValues);
    for (TTestType& v : randomValues)
        v = Random();

    // print out how many values there are, and the value type
    fprintf(file, "static const size_t c_numValues = %u;\n\n", numValues);

    // make a c style array
    fprintf(file, "static const " TTestTypeName " carray[%u] = {", numValues);
    fprintf(file, TTestTypePrintf, randomValues[0]);
    for (size_t index = 1; index < numValues; ++index)
        fprintf(file, ","TTestTypePrintf,randomValues[index]);
    fprintf(file,"};\n\n");

    // make a std::array style array
    fprintf(file, "static const std::array<" TTestTypeName ", %u> stdarray = {", numValues);
    fprintf(file, TTestTypePrintf, randomValues[0]);
    for (size_t index = 1; index < numValues; ++index)
        fprintf(file, ","TTestTypePrintf, randomValues[index]);
    fprintf(file, "}; \n\n");

    // make a std::vector style array
    fprintf(file, "static const std::vector<" TTestTypeName "> stdvector = {");
    fprintf(file, TTestTypePrintf, randomValues[0]);
    for (size_t index = 1; index < numValues; ++index)
        fprintf(file, ","TTestTypePrintf, randomValues[index]);
    fprintf(file, "}; \n\n");

    // make a pointer to dynamic memory (the std::vector's memory!)
    fprintf(file, "static const "TTestTypeName"* dynamicmemory = &stdvector[0]; \n\n");

    // make the switch statement
    fprintf(file, "inline " TTestTypeName " LookupSwitch (size_t index) {\n");
    fprintf(file, "    switch(index) {\n");
    for (size_t index = 0; index < numValues; ++index)
        fprintf(file, "        case %u: return " TTestTypePrintf ";\n", index, randomValues[index]);
    fprintf(file, "    }\n    return -1;\n}\n");

    // close the file
    fclose(file);
}

int main(int argc, char **argv)
{
    // To make the comparison code
    //GenerateCode(5000);

    static const size_t c_times = 1000;

    // do the tests!
    TTestType sum1 = 0;
    {
        SBlockTimer timer("c array sum sequence");
        for (size_t times = 0; times < c_times; ++times)
            for (size_t i = 0; i < c_numValues; ++i)
                sum1 += carray[i];
    }
    TTestType sum2 = 0;
    {
        SBlockTimer timer("std::array sum sequence");
        for (size_t times = 0; times < c_times; ++times)
            for (size_t i = 0; i < c_numValues; ++i)
                sum2 += stdarray[i];
    }
    TTestType sum3 = 0;
    {
        SBlockTimer timer("std::vector sum sequence");
        for (size_t times = 0; times < c_times; ++times)
            for (size_t i = 0; i < c_numValues; ++i)
                sum3 += stdvector[i];
    }
    TTestType sum4 = 0;
    {
        SBlockTimer timer("dynamic memory sum sequence");
        for (size_t times = 0; times < c_times; ++times)
            for (size_t i = 0; i < c_numValues; ++i)
                sum4 += dynamicmemory[i];
    }
    TTestType sum5 = 0;
    {
        SBlockTimer timer("switch sum sequence");
        for (size_t times = 0; times < c_times; ++times)
            for (size_t i = 0; i < c_numValues; ++i)
                sum5 += LookupSwitch(i);
    }

    printf("sums %u\n", sum1 + sum2 + sum3 + sum4 + sum5);
    return 0;
}

/*

TODO:
* measure array access speeds: sequential and random shuffle.
* debug and release
* std::array, std::vector, const c array, dynamic memory (point to vector's memory maybe?), switch statement
* time blocks

! the switch statement is the slowest so far.
 ? maybe its cause it has tests and jumps instead of just memory lookups?
 ? what does this mean for compile time hashing? maybe we need to make a data structure, not a switch statement? it does a strcmp against the string and sets to the enum value if equals, else invalid. nullptr/invalid for invalid entries?

! if i turn the numbers up to 100k, the compiler errors out.
 ? what is limit?
 ? what is limit of switch?

* try making switch inline?

* analyze asm in debug and release? x86/x64?

Next:
* analyze sparse array as switch vs something else

Then:
* hashinator and sparse array utility next!

?use of sparse array?
 * when you are given input but don't care about all of it
 * could also make multiple things point at the same memory if same value!!

* utility programs to generate code from source data that also uses constexpr
* Make it so you can also load data from.a file dynamically, for large data sets.
* See where you can use fpe (encryption) instead of hashing to easily ensure no collisions.
 * Maybe best there for sparse array? not sure how hash table would do

*/