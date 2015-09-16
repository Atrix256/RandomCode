#include <stdio.h>

//=================================================================================
void WaitForEnter()
{
    printf("\nPress Enter to quit");
    fflush(stdin);
    getchar();
}

//=================================================================================
int main (int argc, char **argv)
{
    int x0, x1, k0, k1, k2, k3;

    const int c_maxAmount = 100;

    for (x0 = 0; x0 < c_maxAmount; ++x0)
    {
        printf("%i / %i\n", x0, c_maxAmount);
        for (x1 = 0; x1 < c_maxAmount; ++x1)
            for (k0 = 1; k0 < c_maxAmount; ++k0)
                for (k1 = 1; k1 < c_maxAmount; ++k1)
                    for (k2 = 1; k2 < c_maxAmount; ++k2)
                        for (k3 = 1; k3 < c_maxAmount; ++k3)
                        {
                            if (((x0 % k0) % 2) == 0 &&
                                ((x0 % k1) % 2) == 1 &&
                                ((x0 % k2) % 2) == 0 &&
                                ((x0 % k3) % 2) == 1 &&
                                ((x1 % k0) % 2) == 0 &&
                                ((x1 % k1) % 2) == 0 &&
                                ((x1 % k2) % 2) == 1 &&
                                ((x1 % k3) % 2) == 1
                                /*
                                &&
                                (k0 % 2) == 0 &&
                                (k1 % 2) == 0 && 
                                (k2 % 2) == 0 && 
                                (k3 % 2) == 0
                                */
                                )
                            {
                                printf("%i, %i, %i, %i, %i, %i\n", x0, x1, k0, k1, k2, k3);
                                WaitForEnter();
                            }
                        }
    }

    WaitForEnter();
    return 0;
}

/*
TODO:
? are there any where the keys are all even
 * probably not. I let it run up to 40, for max number = 100 and it didnt find any
* test this in homomorphic setting (n bit adder?)
* generalize program for N bits and whatever highest number search
* multithread it
* try and figure it out mathematically
* after proving that this stuff works homomorphically, gather up what needs to be done, and possible avenues to explore, and present to ben to see if he wants to collaborate.
* make a private github and share w/ him if he wants in


Paper Research TODO's
* find math to solve the constraints.  brute force is slow as f00k
 * look into successive substitution to solve these equations? https://en.wikipedia.org/wiki/Method_of_successive_substitution
* is there a way to have even keys? i don't think so but...
* how about the other way(s) to implement homomorphic encryption.  Does this apply there? are any more interesting / easily done or have better properties?


FIRST ANSWER:
3,7,1,5,3,2

x0 = 3
x1 = 7
k0 = 1
k1 = 5
k2 = 3
k3 = 2

((3 % 1) % 2) == 0
((3 % 5) % 2) == 1
((3 % 3) % 2) == 0
((3 % 2) % 2) == 1
((7 % 1) % 2) == 0
((7 % 5) % 2) == 0
((7 % 3) % 2) == 1
((7 % 2) % 2) == 1

OTHER ANSWER:
((x0 % k0) % 2) == 0
((x0 % k1) % 2) == 1
((x0 % k2) % 2) == 0
((x0 % k3) % 2) == 1
((x1 % k0) % 2) == 0
((x1 % k1) % 2) == 0
((x1 % k2) % 2) == 1
((x1 % k3) % 2) == 1

5, 98, 3, 99, 5, 97

x0 = 5
x1 = 98
k0 = 3
k1 = 99
k2 = 5
k3 = 97

((5 % 3) % 2) == 0
((5 % 99) % 2) == 1
((5 % 5) % 2) == 0
((5 % 97) % 2) == 1
((98 % 3) % 2) == 0
((98 % 99) % 2) == 0
((98 % 5) % 2) == 1
((98 % 97) % 2) == 1
*/