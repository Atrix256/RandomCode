#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!
#include <vector>
#include <stdint.h>
#include <random>
#include <array>
#include <algorithm>
#include <stdlib.h>
#include <set>

typedef uint8_t uint8;

#define NUM_SAMPLES 100  // to simplify some 2d code, this must be a square
#define NUM_SAMPLES_FOR_COLORING 100

// Turning this on will slow things down significantly because it's an O(N^5) operation for 2d!
#define CALCULATE_DISCREPANCY 0

#define IMAGE1D_WIDTH 600
#define IMAGE1D_HEIGHT 50
#define IMAGE2D_WIDTH 300
#define IMAGE2D_HEIGHT 300
#define IMAGE_PAD   30

#define IMAGE1D_CENTERX ((IMAGE1D_WIDTH+IMAGE_PAD*2)/2)
#define IMAGE1D_CENTERY ((IMAGE1D_HEIGHT+IMAGE_PAD*2)/2)
#define IMAGE2D_CENTERX ((IMAGE2D_WIDTH+IMAGE_PAD*2)/2)
#define IMAGE2D_CENTERY ((IMAGE2D_HEIGHT+IMAGE_PAD*2)/2)

#define AXIS_HEIGHT 40
#define DATA_HEIGHT 20
#define DATA_WIDTH 2

#define COLOR_FILL SColor(255,255,255)
#define COLOR_AXIS SColor(0, 0, 0)

//======================================================================================
struct SImageData
{
    SImageData ()
        : m_width(0)
        , m_height(0)
    { }
  
    size_t m_width;
    size_t m_height;
    size_t m_pitch;
    std::vector<uint8> m_pixels;
};

struct SColor
{
    SColor (uint8 _R = 0, uint8 _G = 0, uint8 _B = 0)
        : R(_R), G(_G), B(_B)
    { }

    uint8 B, G, R;
};

//======================================================================================
bool SaveImage (const char *fileName, const SImageData &image)
{
    // open the file if we can
    FILE *file;
    file = fopen(fileName, "wb");
    if (!file) {
        printf("Could not save %s\n", fileName);
        return false;
    }
  
    // make the header info
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER infoHeader;
  
    header.bfType = 0x4D42;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits = 54;
  
    infoHeader.biSize = 40;
    infoHeader.biWidth = (LONG)image.m_width;
    infoHeader.biHeight = (LONG)image.m_height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = (DWORD) image.m_pixels.size();
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;
  
    header.bfSize = infoHeader.biSizeImage + header.bfOffBits;
  
    // write the data and close the file
    fwrite(&header, sizeof(header), 1, file);
    fwrite(&infoHeader, sizeof(infoHeader), 1, file);
    fwrite(&image.m_pixels[0], infoHeader.biSizeImage, 1, file);
    fclose(file);
 
    return true;
}

//======================================================================================
void ImageInit (SImageData& image, size_t width, size_t height)
{
    image.m_width = width;
    image.m_height = height;
    image.m_pitch = 4 * ((width * 24 + 31) / 32);
    image.m_pixels.resize(image.m_pitch * image.m_width);
    std::fill(image.m_pixels.begin(), image.m_pixels.end(), 0);
}

//======================================================================================
void ImageClear (SImageData& image, const SColor& color)
{
    uint8* row = &image.m_pixels[0];
    for (size_t rowIndex = 0; rowIndex < image.m_height; ++rowIndex)
    {
        SColor* pixels = (SColor*)row;
        std::fill(pixels, pixels + image.m_width, color);

        row += image.m_pitch;
    }
}

//======================================================================================
void ImageBox (SImageData& image, size_t x1, size_t x2, size_t y1, size_t y2, const SColor& color)
{
    for (size_t y = y1; y < y2; ++y)
    {
        uint8* row = &image.m_pixels[y * image.m_pitch];
        SColor* start = &((SColor*)row)[x1];
        std::fill(start, start + x2 - x1, color);
    }
}

//======================================================================================
float Distance (float x1, float y1, float x2, float y2)
{
    float dx = (x2 - x1);
    float dy = (y2 - y1);

    return std::sqrtf(dx*dx + dy*dy);
}

//======================================================================================
SColor DataPointColor (size_t sampleIndex)
{
    SColor ret;
    float percent = (float(sampleIndex) / (float(NUM_SAMPLES_FOR_COLORING) - 1.0f));

    ret.R = uint8((1.0f - percent) * 255.0f);
    ret.G = 0;
    ret.B = uint8(percent * 255.0f);

    float mag = (float)sqrt(ret.R*ret.R + ret.G*ret.G + ret.B*ret.B);
    ret.R = uint8((float(ret.R) / mag)*255.0f);
    ret.G = uint8((float(ret.G) / mag)*255.0f);
    ret.B = uint8((float(ret.B) / mag)*255.0f);

    return ret;
}

//======================================================================================
float RandomFloat (float min, float max)
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt);
}

//======================================================================================
size_t Ruler (size_t n)
{
    size_t ret = 0;
    while (n != 0 && (n & 1) == 0)
    {
        n /= 2;
        ++ret;
    }
    return ret;
}

//======================================================================================
float CalculateDiscrepancy1D (const std::array<float, NUM_SAMPLES>& samples)
{
    // some info about calculating discrepancy
    // https://math.stackexchange.com/questions/1681562/how-to-calculate-discrepancy-of-a-sequence

    // Calculates the discrepancy of this data.
    // Assumes the data is [0,1) for valid sample range
    std::array<float, NUM_SAMPLES> sortedSamples = samples;
    std::sort(sortedSamples.begin(), sortedSamples.end());

    float maxDifference = 0.0f;
    for (size_t startIndex = 0; startIndex <= NUM_SAMPLES; ++startIndex)
    {
        // startIndex 0 = 0.0f.  startIndex 1 = sortedSamples[0]. etc

        float startValue = 0.0f;
        if (startIndex > 0)
            startValue = sortedSamples[startIndex - 1];

        for (size_t stopIndex = startIndex; stopIndex <= NUM_SAMPLES; ++stopIndex)
        {
            // stopIndex 0 = sortedSamples[0].  startIndex[N] = 1.0f. etc

            float stopValue = 1.0f;
            if (stopIndex < NUM_SAMPLES)
                stopValue = sortedSamples[stopIndex];

            float length = stopValue - startValue;

            // half open interval [startValue, stopValue)
            float density = (stopIndex - startIndex) / float(NUM_SAMPLES);
            float difference = std::abs(density - length);
            if (difference > maxDifference)
                maxDifference = difference;

            // closed interval [startValue, stopValue]
            density = (stopIndex - startIndex + 1) / float(NUM_SAMPLES);
            difference = std::abs(density - length);
            if (difference > maxDifference)
                maxDifference = difference;
        }
    }
    return maxDifference;
}

//======================================================================================
float CalculateDiscrepancy2D (const std::array<std::array<float, 2>, NUM_SAMPLES>& samples)
{
    // some info about calculating discrepancy
    // https://math.stackexchange.com/questions/1681562/how-to-calculate-discrepancy-of-a-sequence

    // Calculates the discrepancy of this data.
    // Assumes the data is [0,1) for valid sample range.

    // Get the sorted list of unique values on each axis
    std::set<float> setSamplesX;
    std::set<float> setSamplesY;
    for (const std::array<float, 2>& sample : samples)
    {
        setSamplesX.insert(sample[0]);
        setSamplesY.insert(sample[1]);
    }
    std::vector<float> sortedXSamples;
    std::vector<float> sortedYSamples;
    sortedXSamples.reserve(setSamplesX.size());
    sortedYSamples.reserve(setSamplesY.size());
    for (float f : setSamplesX)
        sortedXSamples.push_back(f);
    for (float f : setSamplesY)
        sortedYSamples.push_back(f);

    // Get the sorted list of samples on the X axis, for faster interval testing
    std::array<std::array<float, 2>, NUM_SAMPLES> sortedSamplesX = samples;
    std::sort(sortedSamplesX.begin(), sortedSamplesX.end(),
        [] (const std::array<float, 2>& itemA, const std::array<float, 2>& itemB)
        {
            return itemA[0] < itemB[0];
        }
    );

    // calculate discrepancy
    float maxDifference = 0.0f;
    for (size_t startIndexY = 0; startIndexY <= sortedYSamples.size(); ++startIndexY)
    {
        float startValueY = 0.0f;
        if (startIndexY > 0)
            startValueY = *(sortedYSamples.begin() + startIndexY - 1);

        for (size_t startIndexX = 0; startIndexX <= sortedXSamples.size(); ++startIndexX)
        {
            float startValueX = 0.0f;
            if (startIndexX > 0)
                startValueX = *(sortedXSamples.begin() + startIndexX - 1);

            for (size_t stopIndexY = startIndexY; stopIndexY <= sortedYSamples.size(); ++stopIndexY)
            {
                float stopValueY = 1.0f;
                if (stopIndexY < sortedYSamples.size())
                    stopValueY = sortedYSamples[stopIndexY];

                for (size_t stopIndexX = startIndexX; stopIndexX <= sortedXSamples.size(); ++stopIndexX)
                {
                    float stopValueX = 1.0f;
                    if (stopIndexX < sortedXSamples.size())
                        stopValueX = sortedXSamples[stopIndexX];

                    // calculate area
                    float length = stopValueX - startValueX;
                    float height = stopValueY - startValueY;
                    float area = length * height;

                    // half open interval [startValue, stopValue)
                    size_t countInside = 0;
                    for (const std::array<float, 2>& sample : samples)
                    {
                        if (sample[0] >= startValueX &&
                            sample[1] >= startValueY &&
                            sample[0] < stopValueX &&
                            sample[1] < stopValueY)
                        {
                            ++countInside;
                        }
                    }
                    float density = float(countInside) / float(NUM_SAMPLES);
                    float difference = std::abs(density - area);
                    if (difference > maxDifference)
                        maxDifference = difference;

                    // closed interval [startValue, stopValue]
                    countInside = 0;
                    for (const std::array<float, 2>& sample : samples)
                    {
                        if (sample[0] >= startValueX &&
                            sample[1] >= startValueY &&
                            sample[0] <= stopValueX &&
                            sample[1] <= stopValueY)
                        {
                            ++countInside;
                        }
                    }
                    density = float(countInside) / float(NUM_SAMPLES);
                    difference = std::abs(density - area);
                    if (difference > maxDifference)
                        maxDifference = difference;
                }
            }
        }
    }

    return maxDifference;
}

//======================================================================================
void Test1D (const char* fileName, const std::array<float, NUM_SAMPLES>& samples)
{
    // create and clear the image
    SImageData image;
    ImageInit(image, IMAGE1D_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);

    // setup the canvas
    ImageClear(image, COLOR_FILL);

    // calculate the discrepancy
    #if CALCULATE_DISCREPANCY
        float discrepancy = CalculateDiscrepancy1D(samples);
        printf("%s Discrepancy = %0.2f%%\n", fileName, discrepancy*100.0f);
    #endif

    // draw the sample points
    size_t i = 0;
    for (float f: samples)
    {
        size_t pos = size_t(f * float(IMAGE1D_WIDTH)) + IMAGE_PAD;
        ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i));
        ++i;
    }

    // draw the axes lines. horizontal first then the two vertical
    ImageBox(image, IMAGE_PAD, IMAGE1D_WIDTH + IMAGE_PAD, IMAGE1D_CENTERY, IMAGE1D_CENTERY + 1, COLOR_AXIS);
    ImageBox(image, IMAGE_PAD, IMAGE_PAD + 1, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);
    ImageBox(image, IMAGE1D_WIDTH + IMAGE_PAD, IMAGE1D_WIDTH + IMAGE_PAD + 1, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);

    // save the image
    SaveImage(fileName, image);
}

//======================================================================================
void Test2D (const char* fileName, const std::array<std::array<float,2>, NUM_SAMPLES>& samples)
{
    // create and clear the image
    SImageData image;
    ImageInit(image, IMAGE2D_WIDTH + IMAGE_PAD * 2, IMAGE2D_HEIGHT + IMAGE_PAD * 2);
    
    // setup the canvas
    ImageClear(image, COLOR_FILL);

    // calculate the discrepancy
    #if CALCULATE_DISCREPANCY
        float discrepancy = CalculateDiscrepancy2D(samples);
        printf("%s Discrepancy = %0.2f%%\n", fileName, discrepancy*100.0f);
    #endif

    // draw the sample points
    size_t i = 0;
    for (const std::array<float, 2>& sample : samples)
    {
        size_t posx = size_t(sample[0] * float(IMAGE2D_WIDTH)) + IMAGE_PAD;
        size_t posy = size_t(sample[1] * float(IMAGE2D_WIDTH)) + IMAGE_PAD;
        ImageBox(image, posx - 1, posx + 1, posy - 1, posy + 1, DataPointColor(i));
        ++i;
    }

    // horizontal lines
    ImageBox(image, IMAGE_PAD - 1, IMAGE2D_WIDTH + IMAGE_PAD + 1, IMAGE_PAD - 1, IMAGE_PAD, COLOR_AXIS);
    ImageBox(image, IMAGE_PAD - 1, IMAGE2D_WIDTH + IMAGE_PAD + 1, IMAGE2D_HEIGHT + IMAGE_PAD, IMAGE2D_HEIGHT + IMAGE_PAD + 1, COLOR_AXIS);

    // vertical lines
    ImageBox(image, IMAGE_PAD - 1, IMAGE_PAD, IMAGE_PAD - 1, IMAGE2D_HEIGHT + IMAGE_PAD + 1, COLOR_AXIS);
    ImageBox(image, IMAGE_PAD + IMAGE2D_WIDTH, IMAGE_PAD + IMAGE2D_WIDTH + 1, IMAGE_PAD - 1, IMAGE2D_HEIGHT + IMAGE_PAD + 1, COLOR_AXIS);

    // save the image
    SaveImage(fileName, image);
}

//======================================================================================
void TestUniform1D (bool jitter)
{
    // calculate the sample points
    const float c_cellSize = 1.0f / float(NUM_SAMPLES+1);
    std::array<float, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samples[i] = float(i+1) / float(NUM_SAMPLES+1);
        if (jitter)
            samples[i] += RandomFloat(-c_cellSize*0.5f, c_cellSize*0.5f);
    }

    // save bitmap etc
    if (jitter)
        Test1D("1DUniformJitter.bmp", samples);
    else
        Test1D("1DUniform.bmp", samples);
}

//======================================================================================
void TestUniformRandom1D ()
{
    // calculate the sample points
    const float c_halfJitter = 1.0f / float((NUM_SAMPLES + 1) * 2);
    std::array<float, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
        samples[i] = RandomFloat(0.0f, 1.0f);

    // save bitmap etc
    Test1D("1DUniformRandom.bmp", samples);
}

//======================================================================================
void TestSubRandomA1D (size_t numRegions)
{
    const float c_randomRange = 1.0f / float(numRegions);

    // calculate the sample points
    const float c_halfJitter = 1.0f / float((NUM_SAMPLES + 1) * 2);
    std::array<float, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samples[i] = RandomFloat(0.0f, c_randomRange);
        samples[i] += float(i % numRegions) / float(numRegions);
    }

    // save bitmap etc
    char fileName[256];
    sprintf(fileName, "1DSubRandomA_%zu.bmp", numRegions);
    Test1D(fileName, samples);
}

//======================================================================================
void TestSubRandomB1D ()
{
    // calculate the sample points
    std::array<float, NUM_SAMPLES> samples;
    float sample = RandomFloat(0.0f, 0.5f);
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        sample = std::fmodf(sample + 0.5f + RandomFloat(0.0f, 0.5f), 1.0f);
        samples[i] = sample;
    }

    // save bitmap etc
    Test1D("1DSubRandomB.bmp", samples);
}

//======================================================================================
void TestVanDerCorput (size_t base)
{
    // calculate the sample points
    std::array<float, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samples[i] = 0.0f;
        float denominator = float(base);
        size_t n = i;
        while (n > 0)
        {
            size_t multiplier = n % base;
            samples[i] += float(multiplier) / denominator;
            n = n / base;
            denominator *= base;
        }
    }

    // save bitmap etc
    char fileName[256];
    sprintf(fileName, "1DVanDerCorput_%zu.bmp", base);
    Test1D(fileName, samples);
}

//======================================================================================
void TestIrrational1D (float irrational, float seed)
{
    // calculate the sample points
    std::array<float, NUM_SAMPLES> samples;
    float sample = seed;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        sample = std::fmodf(sample + irrational, 1.0f);
        samples[i] = sample;
    }

    // save bitmap etc
    char irrationalStr[256];
    sprintf(irrationalStr, "%f", irrational);
    char seedStr[256];
    sprintf(seedStr, "%f", seed);
    char fileName[256];
    sprintf(fileName, "1DIrrational_%s_%s.bmp", &irrationalStr[2], &seedStr[2]);
    Test1D(fileName, samples);
}

//======================================================================================
void TestSobol1D ()
{
    // calculate the sample points
    std::array<float, NUM_SAMPLES> samples;
    size_t sampleInt = 0;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        size_t ruler = Ruler(i + 1);
        size_t direction = size_t(size_t(1) << size_t(31 - ruler));
        sampleInt = sampleInt ^ direction;
        samples[i] = float(sampleInt) / std::pow(2.0f, 32.0f);
    }

    // save bitmap etc
    Test1D("1DSobol.bmp", samples);
}

//======================================================================================
void TestHammersley1D (size_t truncateBits)
{
    // calculate the sample points
    std::array<float, NUM_SAMPLES> samples;
    size_t sampleInt = 0;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        size_t n = i >> truncateBits;
        float base = 1.0f / 2.0f;
        samples[i] = 0.0f;
        while (n)
        {
            if (n & 1)
                samples[i] += base;
            n /= 2;
            base /= 2.0f;
        }
    }

    // save bitmap etc
    char fileName[256];
    sprintf(fileName, "1DHammersley_%zu.bmp", truncateBits);
    Test1D(fileName, samples);
}

//======================================================================================
float MinimumDistance1D (const std::array<float, NUM_SAMPLES>& samples, size_t numSamples, float x)
{
    // Used by poisson.
    // This returns the minimum distance that point (x) is away from the sample points, from [0, numSamples).
    float minimumDistance = 0.0f;
    for (size_t i = 0; i < numSamples; ++i)
    {
        float distance = std::abs(samples[i] - x);
        if (i == 0 || distance < minimumDistance)
            minimumDistance = distance;
    }
    return minimumDistance;
}

//======================================================================================
void TestPoisson1D ()
{
    // every time we want to place a point, we generate this many points and choose the one farthest away from all the other points (largest minimum distance)
    const size_t c_bestOfAttempts = 100;

    // calculate the sample points
    std::array<float, NUM_SAMPLES> samples;
    for (size_t sampleIndex = 0; sampleIndex < NUM_SAMPLES; ++sampleIndex)
    {
        // generate some random points and keep the one that has the largest minimum distance from any of the existing points
        float bestX = 0.0f;
        float bestMinDistance = 0.0f;
        for (size_t attempt = 0; attempt < c_bestOfAttempts; ++attempt)
        {
            float attemptX = RandomFloat(0.0f, 1.0f);
            float minDistance = MinimumDistance1D(samples, sampleIndex, attemptX);

            if (minDistance > bestMinDistance)
            {
                bestX = attemptX;
                bestMinDistance = minDistance;
            }
        }
        samples[sampleIndex] = bestX;
    }

    // save bitmap etc
    Test1D("1DPoisson.bmp", samples);
}

//======================================================================================
void TestUniform2D (bool jitter)
{
    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    const size_t c_oneSide = size_t(std::sqrt(NUM_SAMPLES));
    const float c_cellSize = 1.0f / float(c_oneSide+1);
    for (size_t iy = 0; iy < c_oneSide; ++iy)
    {
        for (size_t ix = 0; ix < c_oneSide; ++ix)
        {
            size_t sampleIndex = iy * c_oneSide + ix;

            samples[sampleIndex][0] = float(ix + 1) / (float(c_oneSide + 1));
            if (jitter)
                samples[sampleIndex][0] += RandomFloat(-c_cellSize*0.5f, c_cellSize*0.5f);

            samples[sampleIndex][1] = float(iy + 1) / (float(c_oneSide) + 1.0f);
            if (jitter)
                samples[sampleIndex][1] += RandomFloat(-c_cellSize*0.5f, c_cellSize*0.5f);
        }
    }

    // save bitmap etc
    if (jitter)
        Test2D("2DUniformJitter.bmp", samples);
    else
        Test2D("2DUniform.bmp", samples);
}

//======================================================================================
void TestUniformRandom2D ()
{
    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    const size_t c_oneSide = size_t(std::sqrt(NUM_SAMPLES));
    const float c_halfJitter = 1.0f / float((c_oneSide + 1) * 2);
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samples[i][0] = RandomFloat(0.0f, 1.0f);
        samples[i][1] = RandomFloat(0.0f, 1.0f);
    }

    // save bitmap etc
    Test2D("2DUniformRandom.bmp", samples);
}

//======================================================================================
void TestSubRandomA2D (size_t regionsX, size_t regionsY)
{
    const float c_randomRangeX = 1.0f / float(regionsX);
    const float c_randomRangeY = 1.0f / float(regionsY);

    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samples[i][0] = RandomFloat(0.0f, c_randomRangeX);
        samples[i][0] += float(i % regionsX) / float(regionsX);

        samples[i][1] = RandomFloat(0.0f, c_randomRangeY);
        samples[i][1] += float(i % regionsY) / float(regionsY);
    }

    // save bitmap etc
    char fileName[256];
    sprintf(fileName, "2DSubRandomA_%zu_%zu.bmp", regionsX, regionsY);
    Test2D(fileName, samples);
}

//======================================================================================
void TestSubRandomB2D ()
{
    // calculate the sample points
    float samplex = RandomFloat(0.0f, 0.5f);
    float sampley = RandomFloat(0.0f, 0.5f);
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samplex = std::fmodf(samplex + 0.5f + RandomFloat(0.0f, 0.5f), 1.0f);
        sampley = std::fmodf(sampley + 0.5f + RandomFloat(0.0f, 0.5f), 1.0f);
        samples[i][0] = samplex;
        samples[i][1] = sampley;
    }
    
    // save bitmap etc
    Test2D("2DSubRandomB.bmp", samples);
}

//======================================================================================
void TestHalton (size_t basex, size_t basey)
{
    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    const size_t c_oneSide = size_t(std::sqrt(NUM_SAMPLES));
    const float c_halfJitter = 1.0f / float((c_oneSide + 1) * 2);
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        // x axis
        samples[i][0] = 0.0f;
        {
            float denominator = float(basex);
            size_t n = i;
            while (n > 0)
            {
                size_t multiplier = n % basex;
                samples[i][0] += float(multiplier) / denominator;
                n = n / basex;
                denominator *= basex;
            }
        }

        // y axis
        samples[i][1] = 0.0f;
        {
            float denominator = float(basey);
            size_t n = i;
            while (n > 0)
            {
                size_t multiplier = n % basey;
                samples[i][1] += float(multiplier) / denominator;
                n = n / basey;
                denominator *= basey;
            }
        }
    }

    // save bitmap etc
    char fileName[256];
    sprintf(fileName, "2DHalton_%zu_%zu.bmp", basex, basey);
    Test2D(fileName, samples);
}

//======================================================================================
void TestSobol2D ()
{
    // calculate the sample points

    // x axis
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    size_t sampleInt = 0;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        size_t ruler = Ruler(i + 1);
        size_t direction = size_t(size_t(1) << size_t(31 - ruler));
        sampleInt = sampleInt ^ direction;
        samples[i][0] = float(sampleInt) / std::pow(2.0f, 32.0f);
    }

    // y axis
    // Code adapted from http://web.maths.unsw.edu.au/~fkuo/sobol/
    // uses numbers: new-joe-kuo-6.21201

    // Direction numbers
    std::vector<size_t> V;
    V.resize((size_t)ceil(log((double)NUM_SAMPLES) / log(2.0)));
    V[0] = size_t(1) << size_t(31);
    for (size_t i = 1; i < V.size(); ++i)
        V[i] = V[i - 1] ^ (V[i - 1] >> 1);

    // Samples
    sampleInt = 0;
    for (size_t i = 0; i < NUM_SAMPLES; ++i) {
        size_t ruler = Ruler(i + 1);
        sampleInt = sampleInt ^ V[ruler];
        samples[i][1] = float(sampleInt) / std::pow(2.0f, 32.0f);
    }

    // save bitmap etc
    Test2D("2DSobol.bmp", samples);
}

//======================================================================================
void TestHammersley2D (size_t truncateBits)
{
    // figure out how many bits we are working in.
    size_t value = 1;
    size_t numBits = 0;
    while (value < NUM_SAMPLES)
    {
        value *= 2;
        ++numBits;
    }

    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    size_t sampleInt = 0;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        // x axis
        samples[i][0] = 0.0f;
        {
            size_t n = i >> truncateBits;
            float base = 1.0f / 2.0f;
            while (n)
            {
                if (n & 1)
                    samples[i][0] += base;
                n /= 2;
                base /= 2.0f;
            }
        }

        // y axis
        samples[i][1] = 0.0f;
        {
            size_t n = i >> truncateBits;
            size_t mask = size_t(1) << (numBits - 1 - truncateBits);

            float base = 1.0f / 2.0f;
            while (mask)
            {
                if (n & mask)
                    samples[i][1] += base;
                mask /= 2;
                base /= 2.0f;
            }
        }
    }


    // save bitmap etc
    char fileName[256];
    sprintf(fileName, "2DHammersley_%zu.bmp", truncateBits);
    Test2D(fileName, samples);
}

//======================================================================================
void TestRooks2D ()
{
    // make and shuffle rook positions
    std::random_device rd;
    std::mt19937 mt(rd());
    std::array<size_t, NUM_SAMPLES> rookPositions;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
        rookPositions[i] = i;
    std::shuffle(rookPositions.begin(), rookPositions.end(), mt);

    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        // x axis
        samples[i][0] = float(rookPositions[i]) / float(NUM_SAMPLES-1);

        // y axis
        samples[i][1] = float(i) / float(NUM_SAMPLES - 1);
    }

    // save bitmap etc
    Test2D("2DRooks.bmp", samples);
}

//======================================================================================
void TestIrrational2D (float irrationalx, float irrationaly, float seedx, float seedy)
{
    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    float samplex = seedx;
    float sampley = seedy;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samplex = std::fmodf(samplex + irrationalx, 1.0f);
        sampley = std::fmodf(sampley + irrationaly, 1.0f);

        samples[i][0] = samplex;
        samples[i][1] = sampley;
    }

    // save bitmap etc
    char irrationalxStr[256];
    sprintf(irrationalxStr, "%f", irrationalx);
    char irrationalyStr[256];
    sprintf(irrationalyStr, "%f", irrationaly);
    char seedxStr[256];
    sprintf(seedxStr, "%f", seedx);
    char seedyStr[256];
    sprintf(seedyStr, "%f", seedy);
    char fileName[256];
    sprintf(fileName, "2DIrrational_%s_%s_%s_%s.bmp", &irrationalxStr[2], &irrationalyStr[2], &seedxStr[2], &seedyStr[2]);
    Test2D(fileName, samples);
}

//======================================================================================
float MinimumDistance2D (const std::array<std::array<float, 2>, NUM_SAMPLES>& samples, size_t numSamples, float x, float y)
{
    // Used by poisson.
    // This returns the minimum distance that point (x,y) is away from the sample points, from [0, numSamples).
    float minimumDistance = 0.0f;
    for (size_t i = 0; i < numSamples; ++i)
    {
        float distance = Distance(samples[i][0], samples[i][1], x, y);
        if (i == 0 || distance < minimumDistance)
            minimumDistance = distance;
    }
    return minimumDistance;
}

//======================================================================================
void TestPoisson2D ()
{
    // every time we want to place a point, we generate this many points and choose the one farthest away from all the other points (largest minimum distance)
    const size_t c_bestOfAttempts = 100;

    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    for (size_t sampleIndex = 0; sampleIndex < NUM_SAMPLES; ++sampleIndex)
    {
        // generate some random points and keep the one that has the largest minimum distance from any of the existing points
        float bestX = 0.0f;
        float bestY = 0.0f;
        float bestMinDistance = 0.0f;
        for (size_t attempt = 0; attempt < c_bestOfAttempts; ++attempt)
        {
            float attemptX = RandomFloat(0.0f, 1.0f);
            float attemptY = RandomFloat(0.0f, 1.0f);
            float minDistance = MinimumDistance2D(samples, sampleIndex, attemptX, attemptY);

            if (minDistance > bestMinDistance)
            {
                bestX = attemptX;
                bestY = attemptY;
                bestMinDistance = minDistance;
            }
        }
        samples[sampleIndex][0] = bestX;
        samples[sampleIndex][1] = bestY;
    }

    // save bitmap etc
    Test2D("2DPoisson.bmp", samples);
}

//======================================================================================
int main (int argc, char **argv)
{
    // 1D tests
    {
        TestUniform1D(false);
        TestUniform1D(true);

        TestUniformRandom1D();

        TestSubRandomA1D(2);
        TestSubRandomA1D(4);
        TestSubRandomA1D(8);
        TestSubRandomA1D(16);
        TestSubRandomA1D(32);

        TestSubRandomB1D();

        TestVanDerCorput(2);
        TestVanDerCorput(3);
        TestVanDerCorput(4);
        TestVanDerCorput(5);

        // golden ratio mod 1 aka (sqrt(5) - 1)/2
        TestIrrational1D(0.618034f, 0.0f);
        TestIrrational1D(0.618034f, 0.385180f);
        TestIrrational1D(0.618034f, 0.775719f);
        TestIrrational1D(0.618034f, 0.287194f);

        // sqrt(2) - 1
        TestIrrational1D(0.414214f, 0.0f);
        TestIrrational1D(0.414214f, 0.385180f);
        TestIrrational1D(0.414214f, 0.775719f);
        TestIrrational1D(0.414214f, 0.287194f);

        // PI mod 1
        TestIrrational1D(0.141593f, 0.0f);
        TestIrrational1D(0.141593f, 0.385180f);
        TestIrrational1D(0.141593f, 0.775719f);
        TestIrrational1D(0.141593f, 0.287194f);
        
        TestSobol1D();

        TestHammersley1D(0);
        TestHammersley1D(1);
        TestHammersley1D(2);

        TestPoisson1D();
    }

    // 2D tests
    {
        TestUniform2D(false);
        TestUniform2D(true);

        TestUniformRandom2D();

        TestSubRandomA2D(2, 2);
        TestSubRandomA2D(2, 3);
        TestSubRandomA2D(3, 11);
        TestSubRandomA2D(3, 97);

        TestSubRandomB2D();

        TestHalton(2, 3);
        TestHalton(5, 7);
        TestHalton(13, 9);

        TestSobol2D();

        TestHammersley2D(0);
        TestHammersley2D(1);
        TestHammersley2D(2);

        TestRooks2D();

        // X axis = golden ratio mod 1 aka (sqrt(5)-1)/2
        // Y axis = sqrt(2) mod 1
        TestIrrational2D(0.618034f, 0.414214f, 0.0f, 0.0f);
        TestIrrational2D(0.618034f, 0.414214f, 0.775719f, 0.264045f);

        // X axis = sqrt(2) mod 1
        // Y axis = sqrt(3) mod 1
        TestIrrational2D(std::fmodf((float)std::sqrt(2.0f), 1.0f), std::fmodf((float)std::sqrt(3.0f), 1.0f), 0.0f, 0.0f);
        TestIrrational2D(std::fmodf((float)std::sqrt(2.0f), 1.0f), std::fmodf((float)std::sqrt(3.0f), 1.0f), 0.775719f, 0.264045f);

        TestPoisson2D();
    }

    #if CALCULATE_DISCREPANCY
        printf("\n");
        system("pause");
    #endif
}

/*

BLOG:
* explanation at top should be like
 * "imagine you want to calculate the average of a function"
 * low numbers of samples... random could clump and you don't get a good result.
 * even distribution is good, but you can get aliasing.
 * want something pretty even but still have randomness
 * the randomness turns aliasing into noise
 * maybe also how for instance in this case, uniform sampling is limited to rational numbers (numbers that can be expressed as a fraction), but random sampling doesn't have that limitation.
 * note many types of discrepancy calculations, I'm using star discrepancy (not star anymore)
 * Better example: give player 5 random items, don't want them all to be bad or all good. A good mix usually. ??
? should i show the sequence with fewer points then more and more?
* i tried adding a progressively smaller jitter to van der corput but it seemed to make the sampling worse
* not only is golden ratio a good irrational, it's supposedly the best says wikipedia!  https://en.wikipedia.org/wiki/Low-discrepancy_sequence#Additive_recurrence
* golden ratio, van der corput are nice in that you don't have to know in advance how many samples you want to take.
 * many of these are
* 1D sobol (which is a simplified sobol) is actually van der corput sequence re-arranged a little bit.
* truncating 1D hammersley doesn't make sense. it is a way better idea in 2d since it makes more variation.
* halton is 2d van der corput
* 2D rooks sort of are uniform on one axis (y in my case) and random on x, although the randomness on x is also not quite random since it's a shuffle
* 2D irrational... use sqrt of primes mod 1. (from wikipedia)
 * not good results though. wikipedia says so too so shrug. says to look at PRNGs.
* 2D Hammersley truncation: i couldn't get the same results with truncation as wolfram page did.  Not sure if i'm doing it wrong or they are, can't find another source
 * http://mathworld.wolfram.com/HammersleyPointSet.html
 * i could get either the first three the same or the second 3, but not both.  Makes me think their thing could be wrong.
* Subrandom approaches jittered grid when taken to logical extreme
 * is it also like subrandom is taking away 1 bit of randomness? sorta? but not really....
* Hammersley only fits the whole space if you go through the full power of 2
* 1d Poisson Notes (and 2d?)
 * If doing N samples, need to choose a good minimum distance...
 * too small and the space won't be maximally filled to that distance so will appear random aka high discrepancy.
 * too large and you won't be able to find places for all your samples, so there will be large gaps and high discrepancy.
 * with the perfect value, you'll have N samples all there, and they will be maximally far apart from each other.
 * there are better methods for generating poisson. (link? the thing about generating multiple candidate points and using the best one? mitchels? mitchelsons?)
 * a good place to start: https://www.johndcook.com/blog/2010/06/14/generating-poisson-random-values/

LINKS:
fibanocci colors: http://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/

https://en.m.wikipedia.org/wiki/Halton_sequence

https://en.m.wikipedia.org/wiki/Van_der_Corput_sequence

format preserving encryption post

blue noise posts:
    http://momentsingraphics.de/?p=127
    http://momentsingraphics.de/?p=148
    http://www.joesfer.com/?p=108

sobol:
http://papa.bretmulvey.com/post/153648811993/sobol-sequences-made-simple
http://web.maths.unsw.edu.au/~fkuo/sobol/

hammersley:
http://mathworld.wolfram.com/HammersleyPointSet.html
 
faure:
http://sas.uwaterloo.ca/~dlmcleis/s906/lds.pdf

calculating discrepancy:
https://math.stackexchange.com/questions/1681562/how-to-calculate-discrepancy-of-a-sequence

2d discrepancy info:
https://math.stackexchange.com/questions/2283671/is-it-expected-that-uniform-points-would-have-non-zero-discrepancy/2284163#2284163

random vs uniform
https://math.stackexchange.com/q/425782/138443

sampling
http://gruenschloss.org/

===discrepancy info:===
Matthias Moulin (@matt77hias) tweeted at 7:27 AM on Sat, May 13, 2017:
R. E. Caflisch: Monte Carlo and quasi-Monte Carlo methods https://t.co/f6o1n6o4pk Page 25 contains different discrepancies
(https://twitter.com/matt77hias/status/863400295086403586?s=03)

@marc_b_reynolds FOLLOWS YOU
https://github.com/Marc-B-Reynolds/Stand-alone-junk/blob/master/src/SFH/Sobol.h
while rambling reflected gray codes can be impl. with trailing zero count and small tables. My sobol code does this:

* weyl sequence
http://marc-b-reynolds.github.io/math/2016/02/24/weyl.html

* poisson disc
https://www.jasondavies.com/poisson-disc/

*/