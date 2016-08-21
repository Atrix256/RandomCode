#pragma once

#include <windows.h>
#include <stdio.h>

struct STimer
{
    STimer(const char* label)
        : m_label(label)
    {
        m_lastMessageLength = 0;
        QueryPerformanceFrequency(&m_freq);
        QueryPerformanceCounter(&m_start);
    }

    void ReportProgress(size_t index, size_t max)
    {
        static int lastProgress = -1;
        float percent = float(index) / float(max);
        int progress = int(100.0f * percent);

        if (progress == lastProgress)
            return;
        lastProgress = progress;

        // erase the old message
        printf("\r");
        for (int c = m_lastMessageLength; c >= 0; --c)
            printf(" ");
        printf("\r");

        // Estimate how long remaining based on how long it took us to get to this percent done
        LARGE_INTEGER current;
        QueryPerformanceCounter(&current);
        float currentSeconds = ((float)(current.QuadPart - m_start.QuadPart)) / m_freq.QuadPart;
        float estimatedSeconds = currentSeconds / percent;
        float remainingSeconds = estimatedSeconds - currentSeconds;
        if (remainingSeconds < 0.0f)
            remainingSeconds = 0.0f;

        // convert the time into a more human friendly view, for larger values of time
        const char* remainingTimeUnits = "seconds";
        float remainingTime = remainingSeconds;
        if (remainingTime > 3600.0f)
        {
            remainingTimeUnits = "hours";
            remainingTime /= 3600.0f;
        }
        else if (remainingTime > 60.0f)
        {
            remainingTimeUnits = "minutes";
            remainingTime /= 60.0f;
        }

        // make the new message
        char message[1024];
        sprintf(message, "%i%% (%0.1f %s remaining)", progress, remainingTime, remainingTimeUnits);

        // store off the length of the new message and print it
        m_lastMessageLength = (int)strlen(message);
        printf("%s", message);
    }
 
    ~STimer()
    {
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        float seconds = ((float)(end.QuadPart - m_start.QuadPart)) / m_freq.QuadPart;
        ReportProgress(1, 1);
        printf("\n%s = %0.2f seconds\n", m_label, seconds);
    }
 
    LARGE_INTEGER   m_start;
    LARGE_INTEGER   m_freq;
    const char*     m_label;
    int             m_lastMessageLength;
};