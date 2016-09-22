#include <string.h>
#include <stdio.h>
#include "compile-time-crc32.h"

constexpr unsigned int crc32(const char *s, unsigned int salt = 0) {
    return crc32_rec(salt, s);
}

void Snippet_CompileTimeHashing (void) {

    const char *hello1String = "Hello1";
    unsigned int hashHello1 = crc32(hello1String);  // x86x/x64 - Debug/Release: Run Time.
    unsigned int hashHello2 = crc32("Hello2");      // x86x/x64 - Debug/Release: Run Time.

    const char *hello3String = "Hello3";
    // error C2131: expression did not evaluate to a constant
    //constexpr unsigned int hashHello3 = crc32(hello3String);
    constexpr unsigned int hashHello4 = crc32("Hello4");  // x86x/x64 - Debug: Run Time.  Release: Compile Time

    printf("%X %X %X %X\n", hashHello1, hashHello2, hashHello4, crc32("hello5"));  // x86x/x64 - Debug/Release: Run Time. (!!!)
}

void Snippet_CompileTimeHashSwitching1 (void) {
    unsigned int hash = crc32("Hello1");  // Run Time.
    constexpr unsigned int hashTestHello2 = crc32("Hello2"); // Debug: Run Time. Release: Not calculated at all.
    switch (hash) { // Uses variable on stack
        case hashTestHello2: {  // Compile Time Constant.
            printf("A\n");
            break;
        }
        case crc32("Hello3"): {  // Compile Time Constant.
            printf("B\n");
            break;
        }
        // error C2196: case value '1470747604' already used
        /*
        case crc32("Hello2"): { 
            printf("C\n");
            break;
        }
        */
        default: {
            printf("C\n");
            break;
        }
    }
}

void Snippet_CompileTimeHashSwitching2 (void) {
    // Release: this function just prints "C\n" and exits.  All code melted away at compile time!

    constexpr unsigned int hash = crc32("Hello1");  // Debug: Run Time
    constexpr unsigned int hashTestHello2 = crc32("Hello2"); // Debug: Run Time
    switch (hash) { // Debug: Compile Time Constant.
        case hashTestHello2: {   // Debug: Compile Time Constant.
            printf("A\n");
            break;
        }
        case crc32("Hello3"): {   // Debug: Compile Time Constant.
            printf("B\n");
            break;
        }
        default: {
            printf("C\n");
            break;
        }
    }
}

void Snippet_CompileTimeHashSwitching3 (void) {
    constexpr unsigned int hashTestHello2 = crc32("Hello2"); // Debug: Run Time. Release: Not calculated at all.
    switch (crc32("Hello1")) { // Always Run Time (!!!)
        case hashTestHello2: {  // Compile Time Constant.
            printf("A\n");
            break;
        }
        case crc32("Hello3"): {  // Compile Time Constant.
            printf("B\n");
            break;
        }
        default: {
            printf("C\n");
            break;
        }
    }
}

void Snippet_JumpTable1 () {
    // Debug: Jump Table
    // Release: Just does the constant case, everything else goes away
    unsigned int i = 3;
    switch (i) {
        case 0: printf("A\n"); break;
        case 1: printf("B\n"); break;
        case 2: printf("C\n"); break;
        case 3: printf("D\n"); break;
        case 4: printf("E\n"); break;
        case 5: printf("F\n"); break;
        case 6: printf("G\n"); break;
        case 7: printf("H\n"); break;
        default: printf("None\n"); break;
    }
}

void Snippet_JumpTable2 () {
    // Debug / Release: Does hash and jump table.
    // Note: It AND's against 7 and then tests to see if i is greater than 7 (!!!)
    unsigned int i = crc32("Hello") & 7;
    switch (i) {
        case 0: printf("A\n"); break;
        case 1: printf("B\n"); break;
        case 2: printf("C\n"); break;
        case 3: printf("D\n"); break;
        case 4: printf("E\n"); break;
        case 5: printf("F\n"); break;
        case 6: printf("G\n"); break;
        case 7: printf("H\n"); break;
        default: printf("None\n"); break;
    }
}

void Snippet_JumpTable3 () {
    // Debug: Does hash and jump table.
    // Release: Just prints "A". All other code melted away at compile time.
    constexpr unsigned int i = crc32("Hello") & 7;
    switch (i) {
        case 0: printf("A\n"); break;
        case 1: printf("B\n"); break;
        case 2: printf("C\n"); break;
        case 3: printf("D\n"); break;
        case 4: printf("E\n"); break;
        case 5: printf("F\n"); break;
        case 6: printf("G\n"); break;
        case 7: printf("H\n"); break;
        default: printf("None\n"); break;
    }
}

void Snippet_CompileTimePerfectHashing () {
    // Debug: Does have some sort of jump table setup, despite the cases not being continuous.
    // Release: prints "A\n".  All other code melts away at compile time.
    static const unsigned int c_numBuckets = 16;
    static const unsigned int c_salt = 1337;

    constexpr unsigned int i = crc32("Identifier_A", c_salt) % c_numBuckets;
    switch (i) {
        case (crc32("Identifier_A", c_salt) % c_numBuckets): printf("A\n"); break;
        case (crc32("Identifier_B", c_salt) % c_numBuckets): printf("B\n"); break;
        case (crc32("Identifier_C", c_salt) % c_numBuckets): printf("C\n"); break;
        case (crc32("Identifier_D", c_salt) % c_numBuckets): printf("D\n"); break;
        case (crc32("Identifier_E", c_salt) % c_numBuckets): printf("E\n"); break;
        case (crc32("Identifier_F", c_salt) % c_numBuckets): printf("F\n"); break;
        case (crc32("Identifier_G", c_salt) % c_numBuckets): printf("G\n"); break;
        case (crc32("Identifier_H", c_salt) % c_numBuckets): printf("H\n"); break;
        default: printf("None\n"); break;
    }
}


void Snippet_FindMinimalPerfectHashingSalt ()
{
    // This takes a long time and ends up not finding any salt value that prevents collisions!
    for (unsigned int salt = 1; salt != 0; ++salt) {
        unsigned char mask = 0;
        unsigned int newValue;
        newValue = 1 << (crc32("Identifier_A", salt) % 8);
        if (newValue&mask)
            continue;
        mask |= newValue;
        newValue = 1 << (crc32("Identifier_B", salt) % 8);
        if (newValue&mask)
            continue;
        mask |= newValue;
        newValue = 1 << (crc32("Identifier_C", salt) % 8);
        if (newValue&mask)
            continue;
        mask |= newValue;
        newValue = 1 << (crc32("Identifier_D", salt) % 8);
        if (newValue&mask)
            continue;
        mask |= newValue;
        newValue = 1 << (crc32("Identifier_E", salt) % 8);
        if (newValue&mask)
            continue;
        mask |= newValue;
        newValue = 1 << (crc32("Identifier_F", salt) % 8);
        if (newValue&mask)
            continue;
        mask |= newValue;
        newValue = 1 << (crc32("Identifier_G", salt) % 8);
        if (newValue&mask)
            continue;
        mask |= newValue;
        newValue = 1 << (crc32("Identifier_H", salt) % 8);
        if (newValue&mask)
            continue;
        mask |= newValue;
        printf("Salt = %u!\n", salt);
        return;
    }
    printf("No salt found!\n");
}

void Snippet_CompileTimeMinimalPerfectHash ()
{
    // Debug / Release:
    //   Runs crc32 at runtime only for "i".  The cases are compile time constants as per usual.
    //   Does a jumptable type setup for the switch and does fallthrough to do multiple increments to get the right ID.
    //
    // Release with constexpr on i:
    //   does the printf with a value of 2.  The rest of the code melts away.
    static const unsigned int c_numBuckets = 16;
    static const unsigned int c_salt = 1337;
    static const unsigned int c_invalidID = -1;

    unsigned int i = crc32("Identifier_F", c_salt) % c_numBuckets;
    unsigned int id = c_invalidID;
    switch (i) {
        case (crc32("Identifier_A", c_salt) % c_numBuckets): ++id;
        case (crc32("Identifier_B", c_salt) % c_numBuckets): ++id;
        case (crc32("Identifier_C", c_salt) % c_numBuckets): ++id;
        case (crc32("Identifier_D", c_salt) % c_numBuckets): ++id;
        case (crc32("Identifier_E", c_salt) % c_numBuckets): ++id;
        case (crc32("Identifier_F", c_salt) % c_numBuckets): ++id;
        case (crc32("Identifier_G", c_salt) % c_numBuckets): ++id;
        case (crc32("Identifier_H", c_salt) % c_numBuckets): ++id; 
        // the two lines below are implicit behavior of how this code works
        // break;
        // default: id = c_invalidID; break;
    }

    printf("id = %i\n", id);
}

void Snippet_CompileTimeAssistedStringToEnum ()
{
    // Debug / Release:
    //   Runs crc32 at runtime only for "i".  The cases are compile time constants as per usual.
    //   Does a jumptable type setup for the switch and does a string comparison against the correct string.
    //   If strings are equal, sets the enum value.
    //
    // release when making it constexpr:
    //  Does string compare, and if strings are equal, sets the enum value. (!!!)
    //  Strangely, if i change the strings to "Identifier_FZ" it doesn't do the string compare.
    //  Maybe because it's the wrong length?? Not really sure... (!!!)
    static const unsigned int c_numBuckets = 16;
    static const unsigned int c_salt = 1337;

    const char* testString = "Identifier_F";
    unsigned int i = crc32(testString, c_salt) % c_numBuckets;

    // also useable as an ID
    enum class EIdentifier : unsigned char {
        Identifier_A,
        Identifier_B,
        Identifier_C,
        Identifier_D,
        Identifier_E,
        Identifier_F,
        Identifier_G,
        Identifier_H,
        invalid = -1
    };

    // do hash assisted string comparison
    #define StringCase(x) case (crc32(#x, c_salt) % c_numBuckets) : if(!strcmp(testString, #x)) identifier = EIdentifier::x; else identifier = EIdentifier::invalid; break;
    EIdentifier identifier = EIdentifier::invalid;
    switch (i) {
        StringCase(Identifier_A);
        StringCase(Identifier_B);
        StringCase(Identifier_C);
        StringCase(Identifier_D);
        StringCase(Identifier_E);
        StringCase(Identifier_F);
        StringCase(Identifier_G);
        StringCase(Identifier_H);
        default: identifier = EIdentifier::invalid;
    }
    #undef StringCase

    printf("string translated to enum value %i", identifier);
}

int main(int argc, char **argv)
{
    // Uncomment snippets to see them in action!

    //Snippet_CompileTimeHashing();

    //Snippet_CompileTimeHashSwitching1();
    //Snippet_CompileTimeHashSwitching2();
    //Snippet_CompileTimeHashSwitching3();

    //Snippet_JumpTable1();
    //Snippet_JumpTable2();
    //Snippet_JumpTable3();

    //Snippet_CompileTimePerfectHashing();
    //Snippet_FindMinimalPerfectHashingSalt();

    //Snippet_CompileTimeMinimalPerfectHash();

    Snippet_CompileTimeAssistedStringToEnum();

    return 0;
}

/*

Blog:
* Title: exploring the implications of compile time hashing
* "Never put off til runtime that which can be done at compile time"
* mention "I didn't test the accuracy or speed of this implementation, just am exploring what compile time hashing can do for us"
 * mention warning 'static_cast': truncation of constant value
* link to source: https://gist.github.com/oktal/5573082
* link to github project?
* put this source.cpp file contents inline in the blog post. Or just the snippets that you want to illustrate.
* Link to minimally perfect hash article you wrote up.
* Mention using visual studio 2015 professional. looking at both x86 and x64
 * your compiler or specific code situation might have different results.
 * x86 / x64 seem to have identical behavior
* Also get some insights into how optimizer works
* There are probably some interesting tests I didn't do.  If you think of any and do them, let us know the results! Also, results on different compilers would be interesting to analyze.

* Simple Compile Time Hashing Snippet
 * put code snippet
 * talk about results
 * show assembly to show how to verify what happened at compile time vs runtime

* Switch / Case
 * put each code snippet
 * talk about results for each
 * show assembly to show how to verify what happened at compile time vs runtime
  * Notes
   * Even though you can't really control what is compile time vs not, it can still be useful.
   * nicer than having to make a bunch of static const unsigned ints and then have an if/else if/else if chain.
    * can perhaps even get jump table out of switch statement! (we'll come back to that)
   * strings become like enums, and you get compile time assurance that there aren't collisions.
   * assumes you know all possible input coming in, and that it's valid. (there are ways to validate though, but takes a string compare after the hash)
   * show assembly to show how you can tell if it's happening at compile time or run time.
   ? can we leverage putting a value in a case to force it to be compile time maybe?
   * kind of makes sense how in debug, constexpr stuff is called more - so that you can actually step through it and debug it. Would be nice to be able to turn off to make debug builds faster though?

* Jump Table Stuff
 * put each code snippet and talk about it and assembly.

* Perfect Hashing
 * Show code snippet
 * talk about assembly generated
 * mention the salt and bucket size.
 * couldn't find ANY salt value that made there be no collisions, when having the exact right number of buckets.
  * tried brute force (show snippet)
  * 2^32 aka 4.2 billion values
  * grew the number of buckets til it worked.  Double in this case.
  * The code was still able to do some sort of jump table setup.
 * you get compile time error if there are hash collisions, so you know if it's running, you are safe.
 * great for when you know the full range of string inputs possible and will only get those.

* Minimal Perfect Hashing
 * AKA turn a string into an ID
 * show code snippet
 * compare it to other MPH functions (like the one you already wrote up)
  * Downside: lots of increments if lots of different strings.
  * Workaround: return a literal number instead of incrementing an id.  Macros or templates maybe could help here, to make it not be a manual process of numbering?
 * would need to write a second function to have ID to string conversion
  * Macro lists (link) could be useful to autogenerate both functions from a single source of information (the macro list)

* Compile assisted string to enum
 * aka Mininimal perfect hashing part 2
 * This is basically the same thing as the minimally perfect hash code, except doesn't have the multiple incrementing ID thing (so, is faster for larger numbers of strings to test)
 * could use a macro list to only have to define the enum values in one place
 * This also useful for if you might get invalid string input
 ? is it really faster than other string compare methods?
 ? what about when there are hash collisions for strings?
  ? leave it for someone else to work out?


*/