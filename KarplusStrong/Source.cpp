#include <stdio.h>
#include <memory.h>
#include <inttypes.h>
#include <vector>

// constants
const float c_pi = 3.14159265359f;
const float c_twoPi = 2.0f * c_pi;

// typedefs
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef int16_t     int16;
typedef int32_t     int32;

//this struct is the minimal required header data for a wav file
struct SMinimalWaveFileHeader
{
    //the main chunk
    unsigned char m_chunkID[4];
    uint32		  m_chunkSize;
    unsigned char m_format[4];

    //sub chunk 1 "fmt "
    unsigned char m_subChunk1ID[4];
    uint32		  m_subChunk1Size;
    uint16		  m_audioFormat;
    uint16		  m_numChannels;
    uint32		  m_sampleRate;
    uint32		  m_byteRate;
    uint16		  m_blockAlign;
    uint16		  m_bitsPerSample;

    //sub chunk 2 "data"
    unsigned char m_subChunk2ID[4];
    uint32		  m_subChunk2Size;

    //then comes the data!
};

//this writes
template <typename T>
bool WriteWaveFile(const char *fileName, std::vector<T> data, int16 numChannels, int32 sampleRate)
{
    int32 dataSize = data.size() * sizeof(T);
    int32 bitsPerSample = sizeof(T) * 8;

    //open the file if we can
    FILE *File = nullptr;
    fopen_s(&File, fileName, "w+b");
    if (!File)
        return false;

    SMinimalWaveFileHeader waveHeader;

    //fill out the main chunk
    memcpy(waveHeader.m_chunkID, "RIFF", 4);
    waveHeader.m_chunkSize = dataSize + 36;
    memcpy(waveHeader.m_format, "WAVE", 4);

    //fill out sub chunk 1 "fmt "
    memcpy(waveHeader.m_subChunk1ID, "fmt ", 4);
    waveHeader.m_subChunk1Size = 16;
    waveHeader.m_audioFormat = 1;
    waveHeader.m_numChannels = numChannels;
    waveHeader.m_sampleRate = sampleRate;
    waveHeader.m_byteRate = sampleRate * numChannels * bitsPerSample / 8;
    waveHeader.m_blockAlign = numChannels * bitsPerSample / 8;
    waveHeader.m_bitsPerSample = bitsPerSample;

    //fill out sub chunk 2 "data"
    memcpy(waveHeader.m_subChunk2ID, "data", 4);
    waveHeader.m_subChunk2Size = dataSize;

    //write the header
    fwrite(&waveHeader, sizeof(SMinimalWaveFileHeader), 1, File);

    //write the wave data itself
    fwrite(&data[0], dataSize, 1, File);

    //close the file and return success
    fclose(File);
    return true;
}

template <typename T>
void ConvertFloatSamples (const std::vector<float>& in, std::vector<T>& out)
{
    // make our out samples the right size
    out.resize(in.size());

    // convert in format to out format !
    for (size_t i = 0, c = in.size(); i < c; ++i)
    {
        float v = in[i];
        if (v < 0.0f)
            v *= -float(std::numeric_limits<T>::lowest());
        else
            v *= float(std::numeric_limits<T>::max());
        out[i] = T(v);
    }
}

// returns a value in [0.0f, 1.0f]
float RandomFloat ()
{
    return ((float)rand()) / ((float)RAND_MAX);
}

// returns a value in [-1.0f, 1.0f]
float RandomBipolarFloat()
{
    return RandomFloat() * 2.0f - 1.0f;
}

class CKarplusStrongStringPluck
{
public:
    CKarplusStrongStringPluck (float frequency, float sampleRate, float feedback)
    {
        m_buffer.resize(uint32(float(sampleRate) / frequency));
        for (float &sample : m_buffer)
            sample = RandomBipolarFloat();
        m_index = 0;
        m_feedback = feedback;
    }

    float GenerateSample ()
    {
        // get our sample to return
        float ret = m_buffer[m_index];

        // low pass filter (average) some samples
        float value = (m_buffer[m_index] + m_buffer[(m_index + 1) % m_buffer.size()]) * 0.5f * m_feedback;
        m_buffer[m_index] = value;

        // move to the next sample
        m_index = (m_index + 1) % m_buffer.size();

        // return the sample from the buffer
        return ret;
    }

private:
    std::vector<float>  m_buffer;
    size_t              m_index;
    float               m_feedback;
};

void GenerateSamples (std::vector<float>& samples, int sampleRate)
{
    CKarplusStrongStringPluck note(220.0f, float(sampleRate), 0.996f);
    CKarplusStrongStringPluck note2(330.0f, float(sampleRate), 0.996f);
    CKarplusStrongStringPluck note3(440.0f, float(sampleRate), 0.996f);
    CKarplusStrongStringPluck note4(550.0f, float(sampleRate), 0.996f);

    const int noteTime = sampleRate / 32;

    for (int index = 0, numSamples = samples.size(); index < numSamples; ++index)
    {
        samples[index] = note.GenerateSample();
        
        if (index > noteTime)
            samples[index] += note2.GenerateSample();

        if (index > noteTime*2)
            samples[index] += note3.GenerateSample();

        if (index > noteTime*3)
            samples[index] += note4.GenerateSample();
    }
}

//the entry point of our application
int main(int argc, char **argv)
{
    // sound format parameters
    const int c_sampleRate = 44100;
    const int c_numSeconds = 4;
    const int c_numChannels = 1;
    const int c_numSamples = c_sampleRate * c_numChannels * c_numSeconds;

    // make space for our samples
    std::vector<float> samples;
    samples.resize(c_numSamples);

    // generate samples
    GenerateSamples(samples, c_sampleRate);

    // convert from float to the final format
    std::vector<int32> samplesInt;
    ConvertFloatSamples(samples, samplesInt);

    // write our samples to a wave file
    WriteWaveFile("out.wav", samplesInt, c_numChannels, c_sampleRate);
}

/*

TODO: 
* make algorithm work
* try different LPF's and feedback so it can be compared
* get it workin w/ fractional delay lines?
* make something that plays mutiple notes, like a mini song. end with a chord?
 * maybe some 46 & 2?
* try using something else as a starting state?
 * like a sound sample
 * or a long saw wave!

* Write blog post

http://www.cs.princeton.edu/courses/archive/fall07/cos126/assignments/guitar.html

https://en.wikipedia.org/wiki/Karplus%E2%80%93Strong_string_synthesis

*/