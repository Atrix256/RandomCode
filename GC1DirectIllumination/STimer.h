#pragma once

#include <windows.h>

struct STimer
{
    STimer()
    {
        QueryPerformanceFrequency(&m_freq);
        QueryPerformanceCounter(&m_start);
    }

    size_t ElapsedSeconds ()
    {
        LARGE_INTEGER current;
        QueryPerformanceCounter(&current);
        return (size_t)(((float)(current.QuadPart - m_start.QuadPart)) / (float)m_freq.QuadPart);
    }
 
    LARGE_INTEGER   m_start;
    LARGE_INTEGER   m_freq;
};