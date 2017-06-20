#include <algorithm>
#include <stdlib.h>

size_t BinarySearch8 (size_t needle, const size_t haystack[8])
{
    // using trinary operator
    size_t ret = (haystack[4] <= needle) ? 4 : 0;
    ret += (haystack[ret + 2] <= needle) ? 2 : 0;
    ret += (haystack[ret + 1] <= needle) ? 1 : 0;
    return ret;
}

size_t BinarySearch8b (size_t needle, const size_t haystack[8])
{
    // using multiplication
    size_t ret = (haystack[4] <= needle) * 4;
    ret += (haystack[ret + 2] <= needle) * 2;
    ret += (haystack[ret + 1] <= needle) * 1;
    return ret;
}

size_t BinarySearch7 (size_t needle, const size_t haystack[7])
{
    // non perfect power of 2.  use min() to keep it from going out of bounds.
    size_t ret = 0;
    size_t testIndex = 0;

    // test index is 4, so is within range.
    testIndex = ret + 4;
    ret = (haystack[testIndex] <= needle) ? testIndex : ret;

    // test index is at most 6, so is within range.
    testIndex = ret + 2;
    ret = (haystack[testIndex] <= needle) ? testIndex : ret;

    // test index is at most 7, so could be out of range.
    // use min() to make sure the index stays in range.
    testIndex = std::min<size_t>(ret + 1, 6);
    ret = (haystack[testIndex] <= needle) ? testIndex : ret;

    return ret;
}

int main (int argc, char **argv)
{
    // search a list of size 8
    {
        // show the data
        printf("Seaching through a list with 8 items:\n");
        size_t data[8] = { 1, 3, 5, 6, 9, 11, 15, 21 };
        printf("data = [");
        for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); ++i)
        {
            if (i > 0)
                printf(", ");
            printf("%zu", data[i]);
        }
        printf("]\n");

        // do some searches on it using trinary operation based function
        printf("\nTrinary based searches:\n");
        #define FIND(needle) printf("Find " #needle ": index = %zu, value = %zu, found = %s\n", BinarySearch8(needle, data), data[BinarySearch8(needle, data)], data[BinarySearch8(needle, data)] == needle ? "true" : "false");
        FIND(2);
        FIND(3);
        FIND(0);
        FIND(22);
        FIND(16);
        FIND(15);
        FIND(21);
        #undef FIND

        // do some searches on it using multiplication based function
        printf("\nMultiplication based searches:\n");
        #define FIND(needle) printf("Find " #needle ": index = %zu, value = %zu, found = %s\n", BinarySearch8b(needle, data), data[BinarySearch8b(needle, data)], data[BinarySearch8b(needle, data)] == needle ? "true" : "false");
        FIND(2);
        FIND(3);
        FIND(0);
        FIND(22);
        FIND(16);
        FIND(15);
        FIND(21);
        #undef FIND

        printf("\n\n\n\n");
    }

    // search a list of size 7
    {
        // show the data
        printf("Seaching through a list with 7 items:\n");
        size_t data[7] = { 1, 3, 5, 6, 9, 11, 15};
        printf("data = [");
        for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); ++i)
        {
            if (i > 0)
                printf(", ");
            printf("%zu", data[i]);
        }
        printf("]\n");

        // do some searches on it using trinary operation based function
        printf("\nTrinary based searches:\n");
        #define FIND(needle) printf("Find " #needle ": index = %zu, value = %zu, found = %s\n", BinarySearch7(needle, data), data[BinarySearch7(needle, data)], data[BinarySearch7(needle, data)] == needle ? "true" : "false");
        FIND(2);
        FIND(3);
        FIND(0);
        FIND(22);
        FIND(16);
        FIND(15);
        FIND(21);
        #undef FIND

        printf("\n\n\n\n");
    }

    system("pause");
    return 0;
}

/*

TODO:
* look at assembly?

BLOG:
* post assembly?

*/