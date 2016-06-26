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

void GenerateSamples (std::vector<float>& samples, int sampleRate)
{
    // make mono noise samples
    std::vector<float> noise;
    noise.resize(samples.size() / 2);
    for (float &v : noise)
        v = ((float)rand()) / ((float)RAND_MAX) * 2.0f - 1.0f;

    // calculate some constants
    const int sectionLength = noise.size() / 8;

    // how much shift
    float frequency = 600.0f;
    int shift = int(float(sampleRate) / frequency);

    // generate samples
    for (int index = 0, numSamples = noise.size(); index < numSamples; ++index)
    {
        int section = index / sectionLength;
        int offset = 0;

        switch (section) {
            case 0: offset = int(float(sampleRate) / 200.0f); break;
            case 1: offset = int(float(sampleRate) / 400.0f); break;
            case 2: offset = int(float(sampleRate) / 300.0f); break;
            case 3: offset = int(float(sampleRate) / 800.0f); break;
            case 7: offset = int(float(sampleRate) / 200.0f); break;
            case 6: offset = int(float(sampleRate) / 400.0f); break;
            case 5: offset = int(float(sampleRate) / 300.0f); break;
            case 4: offset = int(float(sampleRate) / 800.0f); break;
        }

        float left = noise[index];
        float right = noise[(index + offset) % numSamples];
        
        samples[index * 2] = left;
        samples[index * 2 + 1] = right;
    }
}

//the entry point of our application
int main(int argc, char **argv)
{
    // generate the stereo whitenoise offset effect
    {
        // sound format parameters
        const int c_sampleRate = 44100;
        const int c_numSeconds = 4;
        const int c_numChannels = 2;
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
        WriteWaveFile("stereonoise.wav", samplesInt, c_numChannels, c_sampleRate);
    }
}

/*

Huggins Binaural Pitch:
 * a shift in period of T results in a tone of 1/T
 * period is recipricol of frequency
  * frequency of 120 beats per minutes (2 beats a second aka 2hz) = 0.5s period.
  * 60 seconds / 120 beats per minute

 * shift = sampleRate / (1000.0 * frequency)


soo...
 * a shift in 0.1 seconds = a tone of 10hz?
 * a shift in 0.5 seconds = a tone of 2hz?
 * a tone of 200hz would = shift of 1/200 seconds = 5ms
 * shift = sampleRate / (1000.0 * frequency)


? what does it sound like when you have white noise offset by small amounts to each ear?

* blog post

links:
http://www.srmathias.com/huggins-pitch/
http://web.mit.edu/hst.723/www/Labs/BinauralDemos.htm


*/