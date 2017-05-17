#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!
#include <vector>
#include <stdint.h>
#include <random>
#include <array>
#include <algorithm>
#include <stdlib.h>

typedef uint8_t uint8;

#define NUM_SAMPLES 100  // to simplify some 2d code, this must be a square
#define NUM_SAMPLES_FOR_COLORING 100

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
template <size_t NumItems>
float CalculateDiscrepancy1D (const std::array<float, NumItems>& samples)
{
    // some info about calculating discrepancy
    // https://math.stackexchange.com/questions/1681562/how-to-calculate-discrepancy-of-a-sequence

    // Calculates the star discrepancy of this data vs the same number of evenly distributed samples.
    // Assumes the data is [0,1).
    std::array<float, NumItems> sortedSamples = samples;
    std::sort(sortedSamples.begin(), sortedSamples.end());

    float maxDifference = 0.0f;
    for (size_t startIndex = 0; startIndex <= sortedSamples.size(); ++startIndex)
    {
        // startIndex 0 = 0.0f.  startIndex 1 = sortedSamples[0]. etc

        float startValue = 0.0f;
        if (startIndex > 0)
            startValue = sortedSamples[startIndex - 1];

        for (size_t stopIndex = startIndex; stopIndex <= sortedSamples.size(); ++stopIndex)
        {
            // stopIndex 0 = sortedSamples[0].  startIndex[N] = 1.0f. etc

            float stopValue = 1.0f;
            if (stopIndex < sortedSamples.size())
                stopValue = sortedSamples[stopIndex];

            float length = stopValue - startValue;

            // half open interval [startValue, stopValue)
            float density = (stopIndex - startIndex) / float(sortedSamples.size());
            float difference = std::abs(density - length);
            if (difference > maxDifference)
                maxDifference = difference;

            // closed interval [startValue, stopValue]
            density = (stopIndex - startIndex + 1) / float(sortedSamples.size());
            difference = std::abs(density - length);
            if (difference > maxDifference)
                maxDifference = difference;
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
    float discrepancy = CalculateDiscrepancy1D(samples);
    printf("%s Discrepancy = %0.2f%%\n", fileName, discrepancy*100.0f);

    // draw the sample points
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        size_t pos = size_t(samples[i] * float(IMAGE1D_WIDTH)) + IMAGE_PAD;
        ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i));
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

    // TODO: calculate discrepancy

    // draw the sample points
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        size_t posx = size_t(samples[i][0] * float(IMAGE2D_WIDTH)) + IMAGE_PAD;
        size_t posy = size_t(samples[i][1] * float(IMAGE2D_WIDTH)) + IMAGE_PAD;
        ImageBox(image, posx - 1, posx + 1, posy - 1, posy + 1, DataPointColor(i));
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
    const float c_halfJitter = 1.0f / float((NUM_SAMPLES + 1) * 2);
    std::array<float, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samples[i] = float(i + 1) / float(NUM_SAMPLES);

        if (jitter)
            samples[i] += RandomFloat(-c_halfJitter, c_halfJitter);
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
void TestSubRandomA1D (size_t numBits)
{
    const size_t c_regions = size_t(1) << numBits;
    const float c_randomRange = 1.0f / float(c_regions);

    // calculate the sample points
    const float c_halfJitter = 1.0f / float((NUM_SAMPLES + 1) * 2);
    std::array<float, NUM_SAMPLES> samples;
    for (size_t i = 0; i < NUM_SAMPLES; ++i)
    {
        samples[i] = RandomFloat(0.0f, c_randomRange);
        samples[i] += float(i % c_regions) / float(c_regions);
    }

    // save bitmap etc
    char fileName[256];
    sprintf(fileName, "1DSubRandomA_%zu.bmp", numBits);
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
void TestUniform2D (bool jitter)
{
    // calculate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    const size_t c_oneSide = size_t(std::sqrt(NUM_SAMPLES));
    const float c_halfJitter = 1.0f / float((c_oneSide + 1) * 2);
    for (size_t iy = 0; iy < c_oneSide; ++iy)
    {
        for (size_t ix = 0; ix < c_oneSide; ++ix)
        {
            size_t sampleIndex = iy * c_oneSide + ix;

            samples[sampleIndex][0] = float(ix + 1) / (float(c_oneSide) + 1.0f);
            if (jitter)
                samples[sampleIndex][0] += RandomFloat(-c_halfJitter, c_halfJitter);

            samples[sampleIndex][1] = float(iy + 1) / (float(c_oneSide) + 1.0f);
            if (jitter)
                samples[sampleIndex][1] += RandomFloat(-c_halfJitter, c_halfJitter);
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
void TestHammersley2D (size_t truncateBits)
{
    // TODO: this isn't using the full height available ?!

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
int main (int argc, char **argv)
{
    // 1D tests
    {
        TestUniform1D(false);
        TestUniform1D(true);

        TestUniformRandom1D();

        TestSubRandomA1D(1);
        TestSubRandomA1D(2);
        TestSubRandomA1D(3);
        TestSubRandomA1D(4);
        TestSubRandomA1D(5);

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

        // TODO: faure sequence
    }

    // 2D tests
    {
        TestUniform2D(false);
        TestUniform2D(true);

        TestUniformRandom2D();

        // TODO: subrandom versions! which lead up to jittered grid

        TestHalton(2, 3);
        TestHalton(5, 7);
        TestHalton(13, 9);

        // TODO: Sobol2d (more involved than 1d!)

        TestHammersley2D(0);
        TestHammersley2D(1);
        TestHammersley2D(2);

        // TODO: temp!
        TestHammersley2D(3);
        TestHammersley2D(4);
        TestHammersley2D(5);

        TestRooks2D();

        // X axis = golden ratio mod 1 aka (sqrt(5)-1)/2
        // Y axis = sqrt(2) mod 1
        TestIrrational2D(0.618034f, 0.414214f, 0.0f, 0.0f);
        TestIrrational2D(0.618034f, 0.414214f, 0.775719f, 0.264045f);

        // sqrt(2) mod 1, sqrt(3) mod 1
        TestIrrational2D(std::fmodf((float)std::sqrt(2.0f), 1.0f), std::fmodf((float)std::sqrt(3.0f), 1.0f), 0.0f, 0.0f);
        TestIrrational2D(std::fmodf((float)std::sqrt(2.0f), 1.0f), std::fmodf((float)std::sqrt(3.0f), 1.0f), 0.775719f, 0.264045f);

        // Poisson?
    }

    printf("\n");
    system("pause");
}

/*

TODO:

* 2d hammersley doesn't look right, thought i fixed it at work but the top is missing samples ?!

? could it be a generalization of hammersley to not always use base 2? sounds more like van der corput though

* BONUS: van der corput based shuffling.
 * can seed (kind of) by changing base
 * 2d shuffle?

? count the bits to make white noise into gaussian? or save this for another post?

* test on x86 and x64!

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

*/