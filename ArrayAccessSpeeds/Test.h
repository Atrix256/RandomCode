#include <stdint.h>
#include <array>
#include <vector>
#include <stdio.h>
#include <chrono>

extern bool   c_verboseSamples;
extern size_t c_testRepeatCount;
extern size_t c_testSamples;

// collects multiple timings of a block of code and calculates the average and standard deviation (variance)
// incremental average and variance algorithm taken from: https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
struct SBlockTimerAggregator {
    SBlockTimerAggregator(const char* label)
        : m_label(label)
        , m_numSamples(0)
        , m_mean(0.0f)
        , m_M2(0.0f)
    {
    }

    void AddSample (float milliseconds)
    {
        ++m_numSamples;
        float delta = milliseconds - m_mean;
        m_mean += delta / float(m_numSamples);
        m_M2 += delta * (milliseconds - m_mean);
    }

    ~SBlockTimerAggregator ()
    {
        printf("%s: avg = %0.2f ms. std dev = %0.2f ms\n", m_label, GetAverage(), GetStandardDeviation());
    }

    float GetAverage () const
    {
        return m_mean;
    }

    float GetVariance () const
    {
        // invalid!
        if (m_numSamples < 2)
            return 0.0f;
        
        return m_M2 / float (m_numSamples - 1);
    }

    float GetStandardDeviation () const
    {
        return sqrt(GetVariance());
    }

    const char* m_label;

    int         m_numSamples;
    float       m_mean;
    float       m_M2;
};

// times a block of code
struct SBlockTimer
{
    SBlockTimer(SBlockTimerAggregator& aggregator)
        : m_aggregator(aggregator)
    {
        m_start = std::chrono::high_resolution_clock::now();
    }

    ~SBlockTimer()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        float milliseconds = seconds.count() * 1000.0f;
        m_aggregator.AddSample(milliseconds);

        if (c_verboseSamples)
            printf("%s %i/%u: %0.2f ms  (avg = %0.2f ms. std dev = %0.2f ms) \n", m_aggregator.m_label, m_aggregator.m_numSamples, c_testSamples, milliseconds, m_aggregator.GetAverage(), m_aggregator.GetStandardDeviation());
    }

    SBlockTimerAggregator&                m_aggregator;
    std::chrono::system_clock::time_point m_start;
};

inline void Fail()
{
    printf("\n\n!!! ERROR !!!\n\n");
    system("pause");
    exit(1);
}