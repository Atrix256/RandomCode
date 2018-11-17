#pragma once

#include <stdio.h>
#include <chrono>

struct STimer
{
    STimer ()
    {
        m_lastMessageLength = 0;
        m_start = std::chrono::high_resolution_clock::now();
    }

    static bool PrettyPrintSeconds (float seconds, char buffer[256])
    {
        // returns true if it ends up showing something other than seconds
        bool ret = false;

        // convert the time into a more human friendly view, for larger values of time
        const char* remainingTimeUnits = "seconds";
        float remainingTime = seconds;
        if (remainingTime > 3600.0f)
        {
            remainingTimeUnits = "hours";
            remainingTime /= 3600.0f;
            ret = true;
        }
        else if (remainingTime > 60.0f)
        {
            remainingTimeUnits = "minutes";
            remainingTime /= 60.0f;
            ret = true;
        }

        // make the string
        sprintf_s(buffer, 256, "%0.1f %s", remainingTime, remainingTimeUnits);
        return ret;
    }

    void ReportProgress (size_t index, size_t max)
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
        std::chrono::duration<float> currentSeconds = std::chrono::high_resolution_clock::now() - m_start;
        float estimatedSeconds = currentSeconds.count() / percent;
        float remainingSeconds = estimatedSeconds - currentSeconds.count();
        if (remainingSeconds < 0.0f)
            remainingSeconds = 0.0f;

        // make the new message
        char timeString[256];
        PrettyPrintSeconds(remainingSeconds, timeString);
        char message[1024];
        sprintf_s(message, 1024, "%0.0f%% (aprox. %s remaining)", 100.0f * percent, timeString);

        // store off the length of the new message and print it
        m_lastMessageLength = (int)strlen(message);
        printf("%s", message);
    }
 
    ~STimer ()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        ReportProgress(1, 1);
        char timeString[256];
        if (PrettyPrintSeconds(seconds.count(), timeString))
            printf("\nRendering took %s (%0.1f seconds)\n", timeString, seconds.count());
        else
            printf("\nRendering took %s\n", timeString);
    }
 
    std::chrono::steady_clock::time_point   m_start;
    int                                     m_lastMessageLength;
};