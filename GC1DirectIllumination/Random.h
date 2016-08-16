#pragma once

#include <stdlib.h>

float RandomFloat ()
{
    // return a random number [0,1]
    return ((float)rand()) / ((float)RAND_MAX);
}