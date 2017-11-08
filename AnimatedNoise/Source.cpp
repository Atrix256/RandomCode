#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <stdint.h>
#include <vector>
#include <random>
#include <atomic>
#include <thread>
#include <complex>
#include <array>

typedef uint8_t uint8;

const float c_pi = 3.14159265359f;

// settings
const bool c_doDFT = true;

// globals 
FILE* g_logFile = nullptr;

//======================================================================================
inline float Lerp (float A, float B, float t)
{
    return A * (1.0f - t) + B * t;
}

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
 
//======================================================================================
struct SColor
{
    SColor (uint8 _R = 0, uint8 _G = 0, uint8 _B = 0)
        : R(_R), G(_G), B(_B)
    { }

    inline void Set (uint8 _R, uint8 _G, uint8 _B)
    {
        R = _R;
        G = _G;
        B = _B;
    }
 
    uint8 B, G, R;
};

//======================================================================================
struct SImageDataComplex
{
    SImageDataComplex ()
        : m_width(0)
        , m_height(0)
    { }
  
    size_t m_width;
    size_t m_height;
    std::vector<std::complex<float>> m_pixels;
};
 
//======================================================================================
std::complex<float> DFTPixel (const SImageData &srcImage, size_t K, size_t L)
{
    std::complex<float> ret(0.0f, 0.0f);
  
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
        {
            // Get the pixel value (assuming greyscale) and convert it to [0,1] space
            const uint8 *src = &srcImage.m_pixels[(y * srcImage.m_pitch) + x * 3];
            float grey = float(src[0]) / 255.0f;
  
            // Add to the sum of the return value
            float v = float(K * x) / float(srcImage.m_width);
            v += float(L * y) / float(srcImage.m_height);
            ret += std::complex<float>(grey, 0.0f) * std::polar<float>(1.0f, -2.0f * c_pi * v);
        }
    }
  
    return ret;
}
  
//======================================================================================
void ImageDFT (const SImageData &srcImage, SImageDataComplex &destImage)
{
    // NOTE: this function assumes srcImage is greyscale, so works on only the red component of srcImage.
    // ImageToGrey() will convert an image to greyscale.
 
    // size the output dft data
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pixels.resize(destImage.m_width*destImage.m_height);
 
    size_t numThreads = std::thread::hardware_concurrency();
    //if (numThreads > 0)
        //numThreads = numThreads - 1;
 
    std::vector<std::thread> threads;
    threads.resize(numThreads);
 
    printf("Doing DFT with %zu threads...\n", numThreads);
 
    // calculate 2d dft (brute force, not using fast fourier transform) multithreadedly
    std::atomic<size_t> nextRow(0);
    for (std::thread& t : threads)
    {
        t = std::thread(
            [&] ()
            {
                size_t row = nextRow.fetch_add(1);
                bool reportProgress = (row == 0);
                int lastPercent = -1;
 
                while (row < srcImage.m_height)
                {
                    // calculate the DFT for every pixel / frequency in this row
                    for (size_t x = 0; x < srcImage.m_width; ++x)
                    {
                        destImage.m_pixels[row * destImage.m_width + x] = DFTPixel(srcImage, x, row);
                    }
 
                    // report progress if we should
                    if (reportProgress)
                    {
                        int percent = int(100.0f * float(row) / float(srcImage.m_height));
                        if (lastPercent != percent)
                        {
                            lastPercent = percent;
                            printf("            \rDFT: %i%%", lastPercent);
                        }
                    }
 
                    // go to the next row
                    row = nextRow.fetch_add(1);
                }
            }
        );
    }
 
    for (std::thread& t : threads)
        t.join();
 
    printf("\n");
}
 
//======================================================================================
void GetMagnitudeData (const SImageDataComplex& srcImage, SImageData& destImage)
{
    // size the output image
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pitch = 4 * ((srcImage.m_width * 24 + 31) / 32);
    destImage.m_pixels.resize(destImage.m_pitch*destImage.m_height);
  
    // get floating point magnitude data
    std::vector<float> magArray;
    magArray.resize(srcImage.m_width*srcImage.m_height);
    float maxmag = 0.0f;
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
        {
            // Offset the information by half width & height in the positive direction.
            // This makes frequency 0 (DC) be at the image origin, like most diagrams show it.
            int k = (x + (int)srcImage.m_width / 2) % (int)srcImage.m_width;
            int l = (y + (int)srcImage.m_height / 2) % (int)srcImage.m_height;
            const std::complex<float> &src = srcImage.m_pixels[l*srcImage.m_width + k];
  
            float mag = std::abs(src);
            if (mag > maxmag)
                maxmag = mag;
  
            magArray[y*srcImage.m_width + x] = mag;
        }
    }
    if (maxmag == 0.0f)
        maxmag = 1.0f;
  
    const float c = 255.0f / log(1.0f+maxmag);
  
    // normalize the magnitude data and send it back in [0, 255]
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
        {
            float src = c * log(1.0f + magArray[y*srcImage.m_width + x]);
  
            uint8 magu8 = uint8(src);
  
            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = magu8;
            dest[1] = magu8;
            dest[2] = magu8;
        }
    }
}

//======================================================================================
bool ImageSave (const SImageData &image, const char *fileName)
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
bool ImageLoad (const char *fileName, SImageData& imageData)
{
    // open the file if we can
    FILE *file;
    file = fopen(fileName, "rb");
    if (!file)
        return false;
 
    // read the headers if we can
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER infoHeader;
    if (fread(&header, sizeof(header), 1, file) != 1 ||
        fread(&infoHeader, sizeof(infoHeader), 1, file) != 1 ||
        header.bfType != 0x4D42 || infoHeader.biBitCount != 24)
    {
        fclose(file);
        return false;
    }
 
    // read in our pixel data if we can. Note that it's in BGR order, and width is padded to the next power of 4
    imageData.m_pixels.resize(infoHeader.biSizeImage);
    fseek(file, header.bfOffBits, SEEK_SET);
    if (fread(&imageData.m_pixels[0], imageData.m_pixels.size(), 1, file) != 1)
    {
        fclose(file);
        return false;
    }
 
    imageData.m_width = infoHeader.biWidth;
    imageData.m_height = infoHeader.biHeight;
    imageData.m_pitch = 4 * ((imageData.m_width * 24 + 31) / 32);
 
    fclose(file);
    return true;
}

//======================================================================================
void ImageInit (SImageData& image, size_t width, size_t height)
{
    image.m_width = width;
    image.m_height = height;
    image.m_pitch = 4 * ((width * 24 + 31) / 32);
    image.m_pixels.resize(image.m_pitch * image.m_height);
    std::fill(image.m_pixels.begin(), image.m_pixels.end(), 0);
}

//======================================================================================
template <typename LAMBDA>
void ImageForEachPixel (SImageData& image, const LAMBDA& lambda)
{
    size_t pixelIndex = 0;
    for (size_t y = 0; y < image.m_height; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];
        for (size_t x = 0; x < image.m_width; ++x)
        {
            lambda(*pixel, pixelIndex);
            ++pixel;
            ++pixelIndex;
        }
    }
}

//======================================================================================
template <typename LAMBDA>
void ImageForEachPixel (const SImageData& image, const LAMBDA& lambda)
{
    size_t pixelIndex = 0;
    for (size_t y = 0; y < image.m_height; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];
        for (size_t x = 0; x < image.m_width; ++x)
        {
            lambda(*pixel, pixelIndex);
            ++pixel;
            ++pixelIndex;
        }
    }
}

//======================================================================================
void ImageConvertToLuma (SImageData& image)
{
    ImageForEachPixel(
        image,
        [] (SColor& pixel, size_t pixelIndex)
        {
            float luma = float(pixel.R) * 0.3f + float(pixel.G) * 0.59f + float(pixel.B) * 0.11f;
            uint8 lumau8 = uint8(luma + 0.5f);
            pixel.R = lumau8;
            pixel.G = lumau8;
            pixel.B = lumau8;
        }
    );
}

//======================================================================================
void ImageCombine2 (const SImageData& imageA, const SImageData& imageB, SImageData& result)
{
    // put the images side by side. A on left, B on right
    ImageInit(result, imageA.m_width + imageB.m_width, max(imageA.m_height, imageB.m_height));
    std::fill(result.m_pixels.begin(), result.m_pixels.end(), 0);

    // image A on left
    for (size_t y = 0; y < imageA.m_height; ++y)
    {
        SColor* destPixel = (SColor*)&result.m_pixels[y * result.m_pitch];
        SColor* srcPixel = (SColor*)&imageA.m_pixels[y * imageA.m_pitch];
        for (size_t x = 0; x < imageA.m_width; ++x)
        {
            destPixel[0] = srcPixel[0];
            ++destPixel;
            ++srcPixel;
        }
    }

    // image B on right
    for (size_t y = 0; y < imageB.m_height; ++y)
    {
        SColor* destPixel = (SColor*)&result.m_pixels[y * result.m_pitch + imageA.m_width * 3];
        SColor* srcPixel = (SColor*)&imageB.m_pixels[y * imageB.m_pitch];
        for (size_t x = 0; x < imageB.m_width; ++x)
        {
            destPixel[0] = srcPixel[0];
            ++destPixel;
            ++srcPixel;
        }
    }
}

//======================================================================================
void ImageCombine3 (const SImageData& imageA, const SImageData& imageB, const SImageData& imageC, SImageData& result)
{
    // put the images side by side. A on left, B in middle, C on right
    ImageInit(result, imageA.m_width + imageB.m_width + imageC.m_width, max(max(imageA.m_height, imageB.m_height), imageC.m_height));
    std::fill(result.m_pixels.begin(), result.m_pixels.end(), 0);

    // image A on left
    for (size_t y = 0; y < imageA.m_height; ++y)
    {
        SColor* destPixel = (SColor*)&result.m_pixels[y * result.m_pitch];
        SColor* srcPixel = (SColor*)&imageA.m_pixels[y * imageA.m_pitch];
        for (size_t x = 0; x < imageA.m_width; ++x)
        {
            destPixel[0] = srcPixel[0];
            ++destPixel;
            ++srcPixel;
        }
    }

    // image B in middle
    for (size_t y = 0; y < imageB.m_height; ++y)
    {
        SColor* destPixel = (SColor*)&result.m_pixels[y * result.m_pitch + imageA.m_width * 3];
        SColor* srcPixel = (SColor*)&imageB.m_pixels[y * imageB.m_pitch];
        for (size_t x = 0; x < imageB.m_width; ++x)
        {
            destPixel[0] = srcPixel[0];
            ++destPixel;
            ++srcPixel;
        }
    }

    // image C on right
    for (size_t y = 0; y < imageC.m_height; ++y)
    {
        SColor* destPixel = (SColor*)&result.m_pixels[y * result.m_pitch + imageA.m_width * 3 + imageC.m_width * 3];
        SColor* srcPixel = (SColor*)&imageC.m_pixels[y * imageC.m_pitch];
        for (size_t x = 0; x < imageC.m_width; ++x)
        {
            destPixel[0] = srcPixel[0];
            ++destPixel;
            ++srcPixel;
        }
    }
}

//======================================================================================
float GoldenRatioMultiple (size_t multiple)
{
    return float(multiple) * (1.0f + std::sqrtf(5.0f)) / 2.0f;
}

//======================================================================================
void IntegrationTest (const SImageData& dither, const SImageData& groundTruth, size_t frameIndex, const char* label)
{
    // calculate min, max, total and average error
    size_t minError = 0;
    size_t maxError = 0;
    size_t totalError = 0;
    size_t pixelCount = 0;
    for (size_t y = 0; y < dither.m_height; ++y)
    {
        SColor* ditherPixel = (SColor*)&dither.m_pixels[y * dither.m_pitch];
        SColor* truthPixel = (SColor*)&groundTruth.m_pixels[y * groundTruth.m_pitch];
        for (size_t x = 0; x < dither.m_width; ++x)
        {
            size_t error = 0;
            if (ditherPixel->R > truthPixel->R)
                error = ditherPixel->R - truthPixel->R;
            else
                error = truthPixel->R - ditherPixel->R;

            totalError += error;

            if ((x == 0 && y == 0) || error < minError)
                minError = error;

            if ((x == 0 && y == 0) || error > maxError)
                maxError = error;

            ++ditherPixel;
            ++truthPixel;
            ++pixelCount;
        }
    }
    float averageError = float(totalError) / float(pixelCount);

    // calculate standard deviation
    float sumSquaredDiff = 0.0f;
    for (size_t y = 0; y < dither.m_height; ++y)
    {
        SColor* ditherPixel = (SColor*)&dither.m_pixels[y * dither.m_pitch];
        SColor* truthPixel = (SColor*)&groundTruth.m_pixels[y * groundTruth.m_pitch];
        for (size_t x = 0; x < dither.m_width; ++x)
        {
            size_t error = 0;
            if (ditherPixel->R > truthPixel->R)
                error = ditherPixel->R - truthPixel->R;
            else
                error = truthPixel->R - ditherPixel->R;

            float diff = float(error) - averageError;

            sumSquaredDiff += diff*diff;
        }
    }
    float stdDev = std::sqrtf(sumSquaredDiff / float(pixelCount - 1));

    // report results
    fprintf(g_logFile, "%s %zu error\n", label, frameIndex);
    fprintf(g_logFile, "  min error: %zu\n", minError);
    fprintf(g_logFile, "  max error: %zu\n", maxError);
    fprintf(g_logFile, "  avg error: %0.2f\n", averageError);
    fprintf(g_logFile, "  stddev: %0.2f\n", stdDev);
    fprintf(g_logFile, "\n");
}

//======================================================================================
template <size_t NUMFRAMES>
void IntegrationTest2 (const SImageData& dither, const SImageData& groundTruth, size_t frameIndex, std::array<size_t, NUMFRAMES>& outMinError, std::array<size_t, NUMFRAMES>& outMaxError, std::array<float, NUMFRAMES>& outAverageError, std::array<float, NUMFRAMES>& outStdDevError)
{
    // calculate min, max, total and average error
    size_t minError = 0;
    size_t maxError = 0;
    size_t totalError = 0;
    size_t pixelCount = 0;
    for (size_t y = 0; y < dither.m_height; ++y)
    {
        SColor* ditherPixel = (SColor*)&dither.m_pixels[y * dither.m_pitch];
        SColor* truthPixel = (SColor*)&groundTruth.m_pixels[y * groundTruth.m_pitch];
        for (size_t x = 0; x < dither.m_width; ++x)
        {
            size_t error = 0;
            if (ditherPixel->R > truthPixel->R)
                error = ditherPixel->R - truthPixel->R;
            else
                error = truthPixel->R - ditherPixel->R;

            totalError += error;

            if ((x == 0 && y == 0) || error < minError)
                minError = error;

            if ((x == 0 && y == 0) || error > maxError)
                maxError = error;

            ++ditherPixel;
            ++truthPixel;
            ++pixelCount;
        }
    }
    float averageError = float(totalError) / float(pixelCount);

    // calculate standard deviation
    float sumSquaredDiff = 0.0f;
    for (size_t y = 0; y < dither.m_height; ++y)
    {
        SColor* ditherPixel = (SColor*)&dither.m_pixels[y * dither.m_pitch];
        SColor* truthPixel = (SColor*)&groundTruth.m_pixels[y * groundTruth.m_pitch];
        for (size_t x = 0; x < dither.m_width; ++x)
        {
            size_t error = 0;
            if (ditherPixel->R > truthPixel->R)
                error = ditherPixel->R - truthPixel->R;
            else
                error = truthPixel->R - ditherPixel->R;

            float diff = float(error) - averageError;

            sumSquaredDiff += diff*diff;
        }
    }
    float stdDev = std::sqrtf(sumSquaredDiff / float(pixelCount - 1));

    outMinError[frameIndex] = minError;
    outMaxError[frameIndex] = maxError;
    outAverageError[frameIndex] = averageError;
    outStdDevError[frameIndex] = stdDev;
}

//======================================================================================
template <size_t NUMFRAMES>
void WriteIntegrationTest2 (std::array<size_t, NUMFRAMES>& minError, std::array<size_t, NUMFRAMES>& maxError, std::array<float, NUMFRAMES>& averageError, std::array<float, NUMFRAMES>& stdDevError, const char* label)
{
    fprintf(g_logFile, "%s error\n", label);
    fprintf(g_logFile, "  min error: ");
    for (size_t v : minError)
        fprintf(g_logFile, "%zu, ", v);

    fprintf(g_logFile, "\n  max error: ");
    for (size_t v : maxError)
        fprintf(g_logFile, "%zu, ", v);

    fprintf(g_logFile, "\n  avg error: ");
    for (float v : averageError)
        fprintf(g_logFile, "%0.2f, ", v);

    fprintf(g_logFile, "\n  stddev: ");
    for (float v : stdDevError)
        fprintf(g_logFile, "%0.2f, ", v);

    fprintf(g_logFile, "\n\n");
}

//======================================================================================
void HistogramTest (const SImageData& noise, size_t frameIndex, const char* label)
{
    std::array<size_t, 256> counts;
    std::fill(counts.begin(), counts.end(), 0);

    ImageForEachPixel(
        noise,
        [&] (const SColor& pixel, size_t pixelIndex)
        {
            counts[pixel.R]++;
        }
    );

    // calculate min, max, total and average
    size_t minCount = 0;
    size_t maxCount = 0;
    size_t totalCount = 0;
    for (size_t i = 0; i < 256; ++i)
    {
        if (i == 0 || counts[i] < minCount)
            minCount = counts[i];

        if (i == 0 || counts[i] > maxCount)
            maxCount = counts[i];

        totalCount += counts[i];
    }
    float averageCount = float(totalCount) / float(256.0f);

    // calculate standard deviation
    float sumSquaredDiff = 0.0f;
    for (size_t i = 0; i < 256; ++i)
    {
        float diff = float(counts[i]) - averageCount;
        sumSquaredDiff += diff*diff;
    }
    float stdDev = std::sqrtf(sumSquaredDiff / 255.0f);

    // report results
    fprintf(g_logFile, "%s %zu histogram\n", label, frameIndex);
    fprintf(g_logFile, "  min count: %zu\n", minCount);
    fprintf(g_logFile, "  max count: %zu\n", maxCount);
    fprintf(g_logFile, "  avg count: %0.2f\n", averageCount);
    fprintf(g_logFile, "  stddev: %0.2f\n", stdDev);
    fprintf(g_logFile, "  counts: ");
    for (size_t i = 0; i < 256; ++i)
    {
        if (i > 0)
            fprintf(g_logFile, ", ");
        fprintf(g_logFile, "%zu", counts[i]);
    }

    fprintf(g_logFile, "\n\n");
}

//======================================================================================
void GenerateWhiteNoise (SImageData& image, size_t width, size_t height)
{
    ImageInit(image, width, height);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<unsigned int> dist(0, 255);

    ImageForEachPixel(
        image,
        [&] (SColor& pixel, size_t pixelIndex)
        {
            uint8 value = dist(rng);
            pixel.R = value;
            pixel.G = value;
            pixel.B = value;
        }
    );
}

//======================================================================================
void GenerateInterleavedGradientNoise (SImageData& image, size_t width, size_t height, float offsetX, float offsetY)
{
    ImageInit(image, width, height);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<unsigned int> dist(0, 255);

    for (size_t y = 0; y < height; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];
        for (size_t x = 0; x < width; ++x)
        {
            float valueFloat = std::fmodf(52.9829189f * std::fmod(0.06711056f*float(x + offsetX) + 0.00583715f*float(y + offsetY), 1.0f), 1.0f);
            size_t valueBig = size_t(valueFloat * 256.0f);
            uint8 value = uint8(valueBig % 256);
            pixel->R = value;
            pixel->G = value;
            pixel->B = value;
            ++pixel;
        }
    }
}

//======================================================================================
template <size_t NUM_SAMPLES>
void GenerateVanDerCoruptSequence (std::array<float, NUM_SAMPLES>& samples, size_t base)
{
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
}

//======================================================================================
void DitherWithTexture (const SImageData& ditherImage, const SImageData& noiseImage, SImageData& result)
{
    // init the result image
    ImageInit(result, ditherImage.m_width, ditherImage.m_height);

    // make the result image
    for (size_t y = 0; y < ditherImage.m_height; ++y)
    {
        SColor* srcDitherPixel = (SColor*)&ditherImage.m_pixels[y * ditherImage.m_pitch];
        SColor* destDitherPixel = (SColor*)&result.m_pixels[y * result.m_pitch];

        for (size_t x = 0; x < ditherImage.m_width; ++x)
        {
            // tile the noise in case it isn't the same size as the image we are dithering
            size_t noiseX = x % noiseImage.m_width;
            size_t noiseY = y % noiseImage.m_height;
            SColor* noisePixel = (SColor*)&noiseImage.m_pixels[noiseY * noiseImage.m_pitch + noiseX * 3];

            uint8 value = 0;
            if (noisePixel->R < srcDitherPixel->R)
                value = 255;

            destDitherPixel->R = value;
            destDitherPixel->G = value;
            destDitherPixel->B = value;

            ++srcDitherPixel;
            ++destDitherPixel;
        }
    }
}

//======================================================================================
void DitherWhiteNoise (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    // make noise
    SImageData noise;
    GenerateWhiteNoise(noise, ditherImage.m_width, ditherImage.m_height);

    // dither the image
    SImageData dither;
    DitherWithTexture(ditherImage, noise, dither);

    // save the results
    SImageData combined;
    ImageCombine3(ditherImage, noise, dither, combined);
    ImageSave(combined, "out/still_whitenoise.bmp");
}

//======================================================================================
void DitherInterleavedGradientNoise (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    // make noise
    SImageData noise;
    GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    // dither the image
    SImageData dither;
    DitherWithTexture(ditherImage, noise, dither);

    // save the results
    SImageData combined;
    ImageCombine3(ditherImage, noise, dither, combined);
    ImageSave(combined, "out/still_ignoise.bmp");
}

//======================================================================================
void DitherBlueNoise (const SImageData& ditherImage, const SImageData& blueNoise)
{
    printf("\n%s\n", __FUNCTION__);

    // dither the image
    SImageData dither;
    DitherWithTexture(ditherImage, blueNoise, dither);

    // save the results
    SImageData combined;
    ImageCombine3(ditherImage, blueNoise, dither, combined);
    ImageSave(combined, "out/still_bluenoise.bmp");
}

//======================================================================================
void DitherWhiteNoiseAnimated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/anim_whitenoise%zu.bmp", i);

        // make noise
        SImageData noise;
        GenerateWhiteNoise(noise, ditherImage.m_width, ditherImage.m_height);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1000.0f);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/anim_ignoise%zu.bmp", i);

        // make noise
        SImageData noise;
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, dist(rng), dist(rng));

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimated (const SImageData& ditherImage, const SImageData blueNoise[8])
{
    printf("\n%s\n", __FUNCTION__);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/anim_bluenoise%zu.bmp", i);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, blueNoise[i], dither);

        // save the results
        SImageData combined;
        ImageCombine2(blueNoise[i], dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherWhiteNoiseAnimatedIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animint_whitenoise%zu.bmp", i);

        // make noise
        SImageData noise;
        GenerateWhiteNoise(noise, ditherImage.m_width, ditherImage.m_height);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimatedIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1000.0f);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animint_ignoise%zu.bmp", i);

        // make noise
        SImageData noise;
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, dist(rng), dist(rng));

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&](SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i + 1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimatedIntegrated (const SImageData& ditherImage, const SImageData blueNoise[8])
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animint_bluenoise%zu.bmp", i);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, blueNoise[i], dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(blueNoise[i], dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherWhiteNoiseAnimatedGoldenRatio (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    // make noise
    SImageData noiseSrc;
    GenerateWhiteNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    SImageDataComplex noiseDFT;
    SImageData noiseDFTMag;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgr_whitenoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = GoldenRatioMultiple(i);
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // DFT the noise
        if (c_doDFT)
        {
            ImageDFT(noise, noiseDFT);
            GetMagnitudeData(noiseDFT, noiseDFTMag);
        }
        else
        {
            ImageInit(noiseDFTMag, noise.m_width, noise.m_height);
            std::fill(noiseDFTMag.m_pixels.begin(), noiseDFTMag.m_pixels.end(), 0);
        }

        // Histogram test the noise
        HistogramTest(noise, i, __FUNCTION__);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // save the results
        SImageData combined;
        ImageCombine3(noiseDFTMag, noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimatedGoldenRatio (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    // make noise
    SImageData noiseSrc;
    GenerateInterleavedGradientNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    SImageDataComplex noiseDFT;
    SImageData noiseDFTMag;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgr_ignoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = GoldenRatioMultiple(i);
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // DFT the noise
        if (c_doDFT)
        {
            ImageDFT(noise, noiseDFT);
            GetMagnitudeData(noiseDFT, noiseDFTMag);
        }
        else
        {
            ImageInit(noiseDFTMag, noise.m_width, noise.m_height);
            std::fill(noiseDFTMag.m_pixels.begin(), noiseDFTMag.m_pixels.end(), 0);
        }

        // Histogram test the noise
        HistogramTest(noise, i, __FUNCTION__);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // save the results
        SImageData combined;
        ImageCombine3(noiseDFTMag, noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimatedGoldenRatio (const SImageData& ditherImage, const SImageData& noiseSrc)
{
    printf("\n%s\n", __FUNCTION__);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    SImageDataComplex noiseDFT;
    SImageData noiseDFTMag;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgr_bluenoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = GoldenRatioMultiple(i);
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // DFT the noise
        if (c_doDFT)
        {
            ImageDFT(noise, noiseDFT);
            GetMagnitudeData(noiseDFT, noiseDFTMag);
        }
        else
        {
            ImageInit(noiseDFTMag, noise.m_width, noise.m_height);
            std::fill(noiseDFTMag.m_pixels.begin(), noiseDFTMag.m_pixels.end(), 0);
        }

        // Histogram test the noise
        HistogramTest(noise, i, __FUNCTION__);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // save the results
        SImageData combined;
        ImageCombine3(noiseDFTMag, noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherWhiteNoiseAnimatedUniform (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    // make noise
    SImageData noiseSrc;
    GenerateWhiteNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    SImageDataComplex noiseDFT;
    SImageData noiseDFTMag;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animuni_whitenoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = float(i) / 8.0f;
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // DFT the noise
        if (c_doDFT)
        {
            ImageDFT(noise, noiseDFT);
            GetMagnitudeData(noiseDFT, noiseDFTMag);
        }
        else
        {
            ImageInit(noiseDFTMag, noise.m_width, noise.m_height);
            std::fill(noiseDFTMag.m_pixels.begin(), noiseDFTMag.m_pixels.end(), 0);
        }

        // Histogram test the noise
        HistogramTest(noise, i, __FUNCTION__);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // save the results
        SImageData combined;
        ImageCombine3(noiseDFTMag, noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimatedUniform (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    // make noise
    SImageData noiseSrc;
    GenerateInterleavedGradientNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    SImageDataComplex noiseDFT;
    SImageData noiseDFTMag;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animuni_ignoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = float(i) / 8.0f;
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // DFT the noise
        if (c_doDFT)
        {
            ImageDFT(noise, noiseDFT);
            GetMagnitudeData(noiseDFT, noiseDFTMag);
        }
        else
        {
            ImageInit(noiseDFTMag, noise.m_width, noise.m_height);
            std::fill(noiseDFTMag.m_pixels.begin(), noiseDFTMag.m_pixels.end(), 0);
        }

        // Histogram test the noise
        HistogramTest(noise, i, __FUNCTION__);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // save the results
        SImageData combined;
        ImageCombine3(noiseDFTMag, noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimatedUniform (const SImageData& ditherImage, const SImageData& noiseSrc)
{
    printf("\n%s\n", __FUNCTION__);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    SImageDataComplex noiseDFT;
    SImageData noiseDFTMag;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animuni_bluenoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = float(i) / 8.0f;
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // DFT the noise
        if (c_doDFT)
        {
            ImageDFT(noise, noiseDFT);
            GetMagnitudeData(noiseDFT, noiseDFTMag);
        }
        else
        {
            ImageInit(noiseDFTMag, noise.m_width, noise.m_height);
            std::fill(noiseDFTMag.m_pixels.begin(), noiseDFTMag.m_pixels.end(), 0);
        }

        // Histogram test the noise
        HistogramTest(noise, i, __FUNCTION__);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // save the results
        SImageData combined;
        ImageCombine3(noiseDFTMag, noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherWhiteNoiseAnimatedGoldenRatioIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // make noise
    SImageData noiseSrc;
    GenerateWhiteNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgrint_whitenoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = GoldenRatioMultiple(i);
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimatedGoldenRatioIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // make noise
    SImageData noiseSrc;
    GenerateInterleavedGradientNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgrint_ignoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = GoldenRatioMultiple(i);
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimatedGoldenRatioIntegrated (const SImageData& ditherImage, const SImageData& noiseSrc)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgrint_bluenoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = GoldenRatioMultiple(i);
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherWhiteNoiseAnimatedUniformIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // make noise
    SImageData noiseSrc;
    GenerateWhiteNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animuniint_whitenoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = float(i) / 8.0f;
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimatedUniformIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // make noise
    SImageData noiseSrc;
    GenerateInterleavedGradientNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animuniint_ignoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = float(i) / 8.0f;
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimatedUniformIntegrated (const SImageData& ditherImage, const SImageData& noiseSrc)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animuniint_bluenoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = float(i) / 8.0f;
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherWhiteNoiseAnimatedVDCIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // make noise
    SImageData noiseSrc;
    GenerateWhiteNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // Make Van Der Corput sequence
    std::array<float, 8> VDC;
    GenerateVanDerCoruptSequence(VDC, 2);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animvdcint_whitenoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = VDC[i];
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimatedVDCIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // make noise
    SImageData noiseSrc;
    GenerateInterleavedGradientNoise(noiseSrc, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // Make Van Der Corput sequence
    std::array<float, 8> VDC;
    GenerateVanDerCoruptSequence(VDC, 2);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animvdcint_ignoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = VDC[i];
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimatedVDCIntegrated (const SImageData& ditherImage, const SImageData& noiseSrc)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    SImageData noise;
    ImageInit(noise, noiseSrc.m_width, noiseSrc.m_height);

    // Make Van Der Corput sequence
    std::array<float, 8> VDC;
    GenerateVanDerCoruptSequence(VDC, 2);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animvdcint_bluenoise%zu.bmp", i);

        // add uniform value to the noise after each frame
        noise.m_pixels = noiseSrc.m_pixels;
        float add = VDC[i];
        ImageForEachPixel(
            noise,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float valueFloat = (float(pixel.R) / 255.0f) + add;
                size_t valueBig = size_t(valueFloat * 255.0f);
                uint8 value = uint8(valueBig % 256);
                pixel.R = value;
                pixel.G = value;
                pixel.B = value;
            }
        );

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest(dither, ditherImage, i, __FUNCTION__);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseOffset1AnimatedIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 8> minError;
    std::array<size_t, 8> maxError;
    std::array<float, 8> averageError;
    std::array<float, 8> stdDevError;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffset_1_ignoise%zu.bmp", i);

        // make noise, offsetting 1 pixel each frame
        SImageData noise;
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, float(i), float(i));

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffset33AnimatedIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 8> minError;
    std::array<size_t, 8> maxError;
    std::array<float, 8> averageError;
    std::array<float, 8> stdDevError;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffset_33_ignoise%zu.bmp", i);

        // make noise, offsetting it each frame
        SImageData noise;
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, float(i)*3.3f, float(i)*3.3f);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffsetGRAnimatedIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 8> minError;
    std::array<size_t, 8> maxError;
    std::array<float, 8> averageError;
    std::array<float, 8> stdDevError;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffset_gr_ignoise%zu.bmp", i);

        // make noise, offsetting it each frame
        SImageData noise;
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, GoldenRatioMultiple(i), GoldenRatioMultiple(i));

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffsetGR8AnimatedIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 8> minError;
    std::array<size_t, 8> maxError;
    std::array<float, 8> averageError;
    std::array<float, 8> stdDevError;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffset_gr8_ignoise%zu.bmp", i);

        // make noise, offsetting it each frame
        SImageData noise;
        float offset = std::floor(9.0f * std::fmodf(GoldenRatioMultiple(i), 1.0f));
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, offset, offset);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffsetVDCAnimatedIntegrated (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 8> minError;
    std::array<size_t, 8> maxError;
    std::array<float, 8> averageError;
    std::array<float, 8> stdDevError;

    // Make Van Der Corput sequence
    std::array<float, 8> VDC;
    GenerateVanDerCoruptSequence(VDC, 2);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffset_vdc_ignoise%zu.bmp", i);

        // make noise, offsetting it each frame
        SImageData noise;
        float offset = std::floor(8.0f * VDC[i]);
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, offset, offset);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 1.0f / float(i+1));

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffset1AnimatedIntegratedDecay (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 60> minError;
    std::array<size_t, 60> maxError;
    std::array<float, 60> averageError;
    std::array<float, 60> stdDevError;

    // animate 60 frames, but only write out the first 8
    for (size_t i = 0; i < 60; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffsetdec_1_ignoise%zu.bmp", i);

        // make noise, offsetting 1 pixel each frame
        SImageData noise;
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, float(i), float(i));

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                if (i == 0)
                    integration[pixelIndex] = pixelValueFloat;
                else
                    integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 0.1f);

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        if (i < 8 || i == 59)
        {
            SImageData combined;
            ImageCombine2(noise, dither, combined);
            ImageSave(combined, fileName);
        }
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffset33AnimatedIntegratedDecay (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 60> minError;
    std::array<size_t, 60> maxError;
    std::array<float, 60> averageError;
    std::array<float, 60> stdDevError;

    // animate 60 frames, but only write out the first 8
    for (size_t i = 0; i < 60; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffsetdec_33_ignoise%zu.bmp", i);

        // make noise, offsetting it each frame
        SImageData noise;
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, float(i)*3.3f, float(i)*3.3f);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                if (i == 0)
                    integration[pixelIndex] = pixelValueFloat;
                else
                    integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 0.1f);

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        if (i < 8 || i == 59)
        {
            SImageData combined;
            ImageCombine2(noise, dither, combined);
            ImageSave(combined, fileName);
        }
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffsetGRAnimatedIntegratedDecay (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 60> minError;
    std::array<size_t, 60> maxError;
    std::array<float, 60> averageError;
    std::array<float, 60> stdDevError;

    // animate 60 frames, but only write out the first 8
    for (size_t i = 0; i < 60; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffsetdec_gr_ignoise%zu.bmp", i);

        // make noise, offsetting it each frame
        SImageData noise;
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, GoldenRatioMultiple(i), GoldenRatioMultiple(i));

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                if (i == 0)
                    integration[pixelIndex] = pixelValueFloat;
                else
                    integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 0.1f);

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        if (i < 8 || i == 59)
        {
            SImageData combined;
            ImageCombine2(noise, dither, combined);
            ImageSave(combined, fileName);
        }
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffsetGR8AnimatedIntegratedDecay (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 60> minError;
    std::array<size_t, 60> maxError;
    std::array<float, 60> averageError;
    std::array<float, 60> stdDevError;

    // animate 60 frames, but only write out the first 8
    for (size_t i = 0; i < 60; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffsetdec_gr8_ignoise%zu.bmp", i);

        // make noise, offsetting it each frame
        SImageData noise;
        float offset = std::floor(9.0f * std::fmodf(GoldenRatioMultiple(i), 1.0f));
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, offset, offset);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                if (i == 0)
                    integration[pixelIndex] = pixelValueFloat;
                else
                    integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 0.1f);

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        if (i < 8 || i == 59)
        {
            SImageData combined;
            ImageCombine2(noise, dither, combined);
            ImageSave(combined, fileName);
        }
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
void DitherInterleavedGradientNoiseOffsetVDCAnimatedIntegratedDecay (const SImageData& ditherImage)
{
    printf("\n%s\n", __FUNCTION__);

    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    std::array<size_t, 60> minError;
    std::array<size_t, 60> maxError;
    std::array<float, 60> averageError;
    std::array<float, 60> stdDevError;

    // Make Van Der Corput sequence
    std::array<float, 8> VDC;
    GenerateVanDerCoruptSequence(VDC, 2);

    // animate 60 frames, but only write out the first 8
    for (size_t i = 0; i < 60; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animintoffsetdec_vdc_ignoise%zu.bmp", i);

        // make noise, offsetting it each frame
        SImageData noise;
        float offset = std::floor(8.0f * VDC[i]);
        GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, offset, offset);

        // dither the image
        SImageData dither;
        DitherWithTexture(ditherImage, noise, dither);

        // integrate and put the current integration results into the dither image
        ImageForEachPixel(
            dither,
            [&] (SColor& pixel, size_t pixelIndex)
            {
                float pixelValueFloat = float(pixel.R) / 255.0f;
                if (i == 0)
                    integration[pixelIndex] = pixelValueFloat;
                else
                    integration[pixelIndex] = Lerp(integration[pixelIndex], pixelValueFloat, 0.1f);

                uint8 integratedPixelValue = uint8(integration[pixelIndex] * 255.0f);
                pixel.R = integratedPixelValue;
                pixel.G = integratedPixelValue;
                pixel.B = integratedPixelValue;
            }
        );

        // do an integration test
        IntegrationTest2(dither, ditherImage, i, minError, maxError, averageError, stdDevError);

        // save the results
        if (i < 8 || i == 59)
        {
            SImageData combined;
            ImageCombine2(noise, dither, combined);
            ImageSave(combined, fileName);
        }
    }

    WriteIntegrationTest2(minError, maxError, averageError, stdDevError, __FUNCTION__);
}

//======================================================================================
int main (int argc, char** argv)
{
    // load the dither image and convert it to greyscale (luma)
    SImageData ditherImage;
    if (!ImageLoad("src/ditherimage.bmp", ditherImage))
    {
        printf("Could not load src/ditherimage.bmp");
        return 0;
    }
    ImageConvertToLuma(ditherImage);

    // load the blue noise images.
    SImageData blueNoise[8];
    for (size_t i = 0; i < 8; ++i)
    {
        char buffer[256];
        sprintf(buffer, "src/BN%zu.bmp", i);
        if (!ImageLoad(buffer, blueNoise[i]))
        {
            printf("Could not load %s", buffer);
            return 0;
        }

        // They have different values in R, G, B so make R be the value for all channels
        ImageForEachPixel(
            blueNoise[i],
            [] (SColor& pixel, size_t pixelIndex)
            {
                pixel.G = pixel.R;
                pixel.B = pixel.R;
            }
        );
    }

    g_logFile = fopen("log.txt", "w+t");

    /*
    // --------------  PART 1: Animating noise over time
    
    // still image dither tests
    DitherWhiteNoise(ditherImage);
    DitherInterleavedGradientNoise(ditherImage);
    DitherBlueNoise(ditherImage, blueNoise[0]);

    // Animated dither tests
    DitherWhiteNoiseAnimated(ditherImage);
    DitherInterleavedGradientNoiseAnimated(ditherImage);
    DitherBlueNoiseAnimated(ditherImage, blueNoise);

    // Golden ratio animated dither tests
    DitherWhiteNoiseAnimatedGoldenRatio(ditherImage);
    DitherInterleavedGradientNoiseAnimatedGoldenRatio(ditherImage);
    DitherBlueNoiseAnimatedGoldenRatio(ditherImage, blueNoise[0]);

    // Uniform animated dither tests
    DitherWhiteNoiseAnimatedUniform(ditherImage);
    DitherInterleavedGradientNoiseAnimatedUniform(ditherImage);
    DitherBlueNoiseAnimatedUniform(ditherImage, blueNoise[0]);

    // Animated dither integration tests
    DitherWhiteNoiseAnimatedIntegrated(ditherImage);
    DitherInterleavedGradientNoiseAnimatedIntegrated(ditherImage);
    DitherBlueNoiseAnimatedIntegrated(ditherImage, blueNoise);

    // Golden ratio animated dither integration tests
    DitherWhiteNoiseAnimatedGoldenRatioIntegrated(ditherImage);
    DitherInterleavedGradientNoiseAnimatedGoldenRatioIntegrated(ditherImage);
    DitherBlueNoiseAnimatedGoldenRatioIntegrated(ditherImage, blueNoise[0]);

    // --------------  PART 2: Uniform over time

    // Uniform animated dither integration tests
    DitherWhiteNoiseAnimatedUniformIntegrated(ditherImage);
    DitherInterleavedGradientNoiseAnimatedUniformIntegrated(ditherImage);
    DitherBlueNoiseAnimatedUniformIntegrated(ditherImage, blueNoise[0]);

    // Van der corput animated dither integration tests
    DitherWhiteNoiseAnimatedVDCIntegrated(ditherImage);
    DitherInterleavedGradientNoiseAnimatedVDCIntegrated(ditherImage);
    DitherBlueNoiseAnimatedVDCIntegrated(ditherImage, blueNoise[0]);

    // --------------  PART 3: Interleaved Gradient Noise 

    // offsets
    DitherInterleavedGradientNoiseOffset1AnimatedIntegrated(ditherImage);
    DitherInterleavedGradientNoiseOffset33AnimatedIntegrated(ditherImage);
    DitherInterleavedGradientNoiseOffsetGRAnimatedIntegrated(ditherImage);
    DitherInterleavedGradientNoiseOffsetGR8AnimatedIntegrated(ditherImage);
    DitherInterleavedGradientNoiseOffsetVDCAnimatedIntegrated(ditherImage);
    */

    // exponential decay offsets
    DitherInterleavedGradientNoiseOffset1AnimatedIntegratedDecay(ditherImage);
    DitherInterleavedGradientNoiseOffset33AnimatedIntegratedDecay(ditherImage);
    DitherInterleavedGradientNoiseOffsetGRAnimatedIntegratedDecay(ditherImage);
    DitherInterleavedGradientNoiseOffsetGR8AnimatedIntegratedDecay(ditherImage);
    DitherInterleavedGradientNoiseOffsetVDCAnimatedIntegratedDecay(ditherImage);

    fclose(g_logFile);

    return 0;
}

/*

IG Noise stuff:

! you didn't change the for loops to go until 60 . the graphs to 60 are invalid! fix and get last frame images and graphs to 60.
* i think i should do like a 60 frame analasys of the decay things? no need for animation, just do to see how it converges after 1 second.
 * could show every 5 or 10 images or something maybe? or just last frame perhaps
 * yeah last frame seems reasonable
* uncomment the code you commented here to make it run faster

Jorge:
 * share animations and graphs
 * starting at (0,0) of IGN.
 ? is an exponential value of 0.9 reasonable / decent?
  * is it also ok on i = 0 to take the first sample and use it w/o lerp?
 ? what is "NoiseTemporalDither(-1,1)" ? from function you shared
 ? "8 uniform steps, then some random noise as you did, but small offset, and possibly horizontally to get new values outside of the diaagonal"
 ? how would adding the 1/8 scaled blue noise work.

Data Observations:
 * Non decay in 8 frames, VDC seems to be the winner!
 * Decay 0.9, around frame 44 the variance of error goes to zero but error jumps up. uniform sampling? ):

* BLOG:
 * show major and minor axis of noise.
 * "that is why it converges better than blue noise because it's just perfectly uniform and allows to get 8 steps" - jorge about diagonal
 * "Forgotten to mention that IGN has a very sough-after property for TAA: for a given pixel if you take the 3x3 neighborhood, it pretty much covers the whole noise dynamic range, meaning the neighborhood clamp will do a great job at preserving the history buffer."


Next blog post:
* taking 2d blue noise, normalizing histogram of each row, seeing how that does for integration & show the 1d DFT's!
* scale it to 3d blue noise and try the same?

*/