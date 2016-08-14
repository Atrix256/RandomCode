#pragma once

#include <windows.h>
#include <stdio.h>

struct STimer
{
    STimer()
    {
        QueryPerformanceFrequency(&m_freq);
        QueryPerformanceCounter(&m_start);
    }
 
    ~STimer()
    {
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        float seconds = ((float)(end.QuadPart - m_start.QuadPart)) / m_freq.QuadPart;
        printf("%0.2f seconds\n", seconds);
    }
 
    LARGE_INTEGER m_start;
    LARGE_INTEGER m_freq;
};