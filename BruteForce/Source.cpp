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
* test this in homomorphic setting (adder?)
* generalize program for N bits and whatever highest number search
* multithread it
* try and figure it out mathematically
* dump it to a text file and look for smaller values just to see if there are any?

AN ANSWER:
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