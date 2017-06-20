#include <algorithm>
#include <stdlib.h>

size_t BinarySearch8 (size_t needle, size_t haystack[8])
{
    // using trinary operator
    size_t ret = (haystack[4] <= needle) ? 4 : 0;
    ret += (haystack[ret + 2] <= needle) ? 2 : 0;
    ret += (haystack[ret + 1] <= needle) ? 1 : 0;
    return ret;
}

size_t BinarySearch8b (size_t needle, size_t haystack[8])
{
    // using multiplication
    size_t ret = (haystack[4] <= needle) * 4;
    ret += (haystack[ret + 2] <= needle) * 2;
    ret += (haystack[ret + 1] <= needle) * 1;
    return ret;
}

size_t BinarySearch7 (size_t needle, size_t haystack[7])
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
    size_t array8[] = { 1, 3, 5, 6, 9, 11, 15, 21 };
    //size_t array7[] = { 1, 3, 5, 6, 9, 11, 15 };

    size_t indexA = BinarySearch8(2, array8);
    size_t indexB = BinarySearch8(3, array8);
    /*
    size_t indexC = BinarySearch8(4, array8);
    size_t indexD = BinarySearch8(0, array8);
    size_t indexE = BinarySearch8(22, array8);
    size_t indexF = BinarySearch8(7, array8);
    size_t indexG = BinarySearch8(10, array8);
    size_t indexH = BinarySearch8(14, array8);
    size_t indexI = BinarySearch8(18, array8);

    size_t indexA2 = BinarySearch7(2, array7);
    size_t indexB2 = BinarySearch7(16, array7);
    */

    printf("%zu\n", indexA);
    printf("%zu\n", indexB);
    system("pause");
    return 0;
}

/*

TODO:
* look at assembly?

BLOG:
1) Binary search perfect power of 2
1b) show without trinary operator usage
2) Binary search non perfect power of 2 (std::min to keep in range)
3) As is, it finds the beginning of the range of where the value falls, except for the case of it being less than all. Could also another comparison to check to see if it's there.
* SIMD / GPU efficient because there are no if statements or variable number of loops.
* mention that binary search isn't the most cache efficient.
 * maybe could be combined with this: http://bannalia.blogspot.com/2015/06/cache-friendly-binary-search.html
* post assembly?


*/