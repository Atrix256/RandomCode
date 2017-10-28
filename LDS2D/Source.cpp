#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <vector>
#include <stdint.h>
#include <random>
#include <algorithm>
#include <complex>
#include <thread>
#include <atomic>
#include <array>

typedef uint8_t uint8;
typedef int64_t int64;

const float c_pi = 3.14159265359f;

const float c_goldenRatioConjugate = 1.61803398875f;

// settings to speed things up when iterating
#define IMAGE_DOWNSIZE_FACTOR() 1
#define DO_DFT() true // TODO: set this to true before checking in

FILE* s_logFile = nullptr;

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

SImageData s_stippleImage;
 
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
void DFTImage (const SImageData &srcImage, SImageDataComplex &destImage)
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
void GetPhaseData (const SImageDataComplex& srcImage, SImageData& destImage)
{
    // size the output image
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pitch = 4 * ((srcImage.m_width * 24 + 31) / 32);
    destImage.m_pixels.resize(destImage.m_pitch*destImage.m_height);
 
    // get floating point phase data, and encode it in [0,255]
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
        {
            // Offset the information by half width & height in the positive direction.
            // This makes frequency 0 (DC) be at the image origin, like most diagrams show it.
            int k = (x + (int)srcImage.m_width / 2) % (int)srcImage.m_width;
            int l = (y + (int)srcImage.m_height / 2) % (int)srcImage.m_height;
            const std::complex<float> &src = srcImage.m_pixels[l*srcImage.m_width + k];
 
            // get phase, and change it from [-pi,+pi] to [0,255]
            float phase = (0.5f + 0.5f * std::atan2(src.real(), src.imag()) / c_pi);
            if (phase < 0.0f)
                phase = 0.0f;
            if (phase > 1.0f)
                phase = 1.0;
            uint8 phase255 = uint8(phase * 255);
 
            // write the phase as grey scale color
            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = phase255;
            dest[1] = phase255;
            dest[2] = phase255;
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
void ImageTile (const SImageData& srcImageData, SImageData& destImageData, size_t width, size_t height)
{
    for (size_t y = 0; y < destImageData.m_height; ++y)
    {
        size_t srcY = y % width;

        for (size_t x = 0; x < destImageData.m_width; ++x)
        {
            size_t srcX = x % height;

            SColor* srcPixel = (SColor*)&srcImageData.m_pixels[srcY * srcImageData.m_pitch + srcX * 3];
            SColor* destPixel = (SColor*)&destImageData.m_pixels[y * destImageData.m_pitch + x * 3];

            *destPixel = *srcPixel;
        }
    }
}

//======================================================================================
void GradientTest (const SImageData& noiseImage, const char* fileName)
{
    SImageData outputImage;
    ImageInit(outputImage, noiseImage.m_width, noiseImage.m_height);

    for (size_t y = 0; y < noiseImage.m_height; ++y)
    {
        for (size_t x = 0; x < noiseImage.m_width; ++x)
        {
            float gradientPixel = 255.0f * float(x) / float(noiseImage.m_width);

            SColor* noisePixel = (SColor*)&noiseImage.m_pixels[y * noiseImage.m_pitch + x * 3];
            SColor* destPixel = (SColor*)&outputImage.m_pixels[y * outputImage.m_pitch + x * 3];

            if (noisePixel->R >= gradientPixel)
                destPixel->Set(0, 0, 0);
            else
                destPixel->Set(255, 255, 255);
        }
    }

    // save the image
    char outFileName[256];
    strcpy(outFileName, fileName);
    strcat(outFileName, ".gradient.bmp");
    ImageSave(outputImage, outFileName);
}

//======================================================================================
void StippleTest (const SImageData& noiseImage, const char* fileName)
{
    SImageData outputImage;
    ImageInit(outputImage, s_stippleImage.m_width, s_stippleImage.m_height);

    std::vector<int64> pixelDifferences;
    pixelDifferences.resize(s_stippleImage.m_width * s_stippleImage.m_height);

    for (size_t y = 0; y < s_stippleImage.m_height; ++y)
    {
        for (size_t x = 0; x < s_stippleImage.m_width; ++x)
        {
            SColor* srcPixel = (SColor*)&s_stippleImage.m_pixels[y * s_stippleImage.m_pitch + x * 3];
            SColor* destPixel = (SColor*)&outputImage.m_pixels[y * outputImage.m_pitch + x * 3];

            size_t noiseX = x % noiseImage.m_width;
            size_t noiseY = y % noiseImage.m_height;

            SColor* noisePixel = (SColor*)&noiseImage.m_pixels[noiseY * noiseImage.m_pitch + noiseX * 3];

            if (noisePixel->R >= srcPixel->R)
                destPixel->Set(0, 0, 0);
            else
                destPixel->Set(255, 255, 255);

            pixelDifferences[y * s_stippleImage.m_width + x] = int64(destPixel->R) - int64(srcPixel->R);
        }
    }

    // calculate some metrics
    int64 totalDiff = 0;
    int64 totalAbsDiff = 0;
    for (int64 v : pixelDifferences)
    {
        totalDiff += v;
        totalAbsDiff += std::abs(v);
    }
    float averageDiff = float(totalDiff) / float(pixelDifferences.size());
    float stdDev = 0.0f;
    for (int64 v : pixelDifferences)
    {
        stdDev += (float(v) - averageDiff) * (float(v) - averageDiff);
    }
    stdDev = std::sqrt(stdDev / float(pixelDifferences.size()));

    fprintf(s_logFile, "%s\nTotal Diff: %zi\nTotal Abs Diff: %zi\nAverage Diff:%0.2f\nStdDev. Diff: %0.2f\n\n", fileName, totalDiff, totalAbsDiff, averageDiff, stdDev);

    // save the image
    char outFileName[256];
    strcpy(outFileName, fileName);
    strcat(outFileName, ".stipple.bmp");
    ImageSave(outputImage, outFileName);

    // also do a gradient test
    GradientTest(noiseImage, fileName);
}

//======================================================================================
// Idea 1: Do some shuffles on X axis(try different sizes).  Add golden ratio on y.
void Idea1 (size_t imageSize, const char* fileName, size_t shuffleSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());

    // make the shuffles for x axis
    std::vector<float> shuffleItemsX;
    shuffleItemsX.resize(imageSize);
    for (size_t i = 0; i < imageSize; ++i)
        shuffleItemsX[i] = float(i % shuffleSize) / float(shuffleSize - 1);
    
    // shuffle each section individually
    size_t shuffleStart = 0;
    while (shuffleStart < imageSize)
    {
        size_t shuffleEnd = min(shuffleStart + shuffleSize, imageSize);
        std::shuffle(shuffleItemsX.begin() + shuffleStart, shuffleItemsX.begin() + shuffleEnd, rng);
        shuffleStart += shuffleSize;
    }

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        float goldenRatioAmount = std::fmod(y * c_goldenRatioConjugate, 1.0f);

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = shuffleItemsX[x] + goldenRatioAmount;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Idea 2: Do some shuffles on X axis (try different sizes). Do modulus of shuffle size on y * golden ratio to make the y axis "repeat" as often as x axis?
void Idea2 (size_t imageSize, const char* fileName, size_t shuffleSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());

    // make the shuffles for x axis
    std::vector<float> shuffleItemsX;
    shuffleItemsX.resize(imageSize);
    for (size_t i = 0; i < imageSize; ++i)
        shuffleItemsX[i] = float(i % shuffleSize) / float(shuffleSize - 1);
    
    // shuffle each section individually
    size_t shuffleStart = 0;
    while (shuffleStart < imageSize)
    {
        size_t shuffleEnd = min(shuffleStart + shuffleSize, imageSize);
        std::shuffle(shuffleItemsX.begin() + shuffleStart, shuffleItemsX.begin() + shuffleEnd, rng);
        shuffleStart += shuffleSize;
    }

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        float goldenRatioAmount = std::fmod(float(y%shuffleSize) * c_goldenRatioConjugate, 1.0f);

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = shuffleItemsX[x] + goldenRatioAmount;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Idea 3: Make Shuffles for X and Y.  add the numbers together to figure out how much to multiply golden ratio by.
void Idea3 (size_t imageSize, const char* fileName, size_t shuffleSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());

    // make the shuffles for x and y axis
    std::vector<float> shuffleItemsX;
    std::vector<float> shuffleItemsY;
    shuffleItemsX.resize(imageSize);
    shuffleItemsY.resize(imageSize);
    for (size_t i = 0; i < imageSize; ++i)
    {
        shuffleItemsX[i] = float(i % shuffleSize) / float(shuffleSize - 1);
        shuffleItemsY[i] = float(i % shuffleSize) / float(shuffleSize - 1);
    }
    
    // shuffle each section individually
    size_t shuffleStart = 0;
    while (shuffleStart < imageSize)
    {
        size_t shuffleEnd = min(shuffleStart + shuffleSize, imageSize);
        std::shuffle(shuffleItemsX.begin() + shuffleStart, shuffleItemsX.begin() + shuffleEnd, rng);
        std::shuffle(shuffleItemsY.begin() + shuffleStart, shuffleItemsY.begin() + shuffleEnd, rng);
        shuffleStart += shuffleSize;
    }

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = std::fmod((shuffleItemsX[x] + shuffleItemsY[y])*c_goldenRatioConjugate, 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Idea 4: Make Shuffles for X and Y.  multiply the numbers together to figure out how much to multiply golden ratio by.
void Idea4 (size_t imageSize, const char* fileName, size_t shuffleSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());

    // make the shuffles for x and y axis
    std::vector<float> shuffleItemsX;
    std::vector<float> shuffleItemsY;
    shuffleItemsX.resize(imageSize);
    shuffleItemsY.resize(imageSize);
    for (size_t i = 0; i < imageSize; ++i)
    {
        shuffleItemsX[i] = float(i % shuffleSize) / float(shuffleSize - 1);
        shuffleItemsY[i] = float(i % shuffleSize) / float(shuffleSize - 1);
    }
    
    // shuffle each section individually
    size_t shuffleStart = 0;
    while (shuffleStart < imageSize)
    {
        size_t shuffleEnd = min(shuffleStart + shuffleSize, imageSize);
        std::shuffle(shuffleItemsX.begin() + shuffleStart, shuffleItemsX.begin() + shuffleEnd, rng);
        std::shuffle(shuffleItemsY.begin() + shuffleStart, shuffleItemsY.begin() + shuffleEnd, rng);
        shuffleStart += shuffleSize;
    }

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = std::fmod((shuffleItemsX[x] * shuffleItemsY[y])*c_goldenRatioConjugate, 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Idea 5: (x+y)*golden ratio
void Idea5 (size_t imageSize, const char* fileName)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = std::fmod((x+y)*c_goldenRatioConjugate, 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Idea 6: Random seed for each row and column. pixel = (seed for row + y * golden ratio) + (seed for column + x * golden ratio)
void Idea6 (size_t imageSize, const char* fileName)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0, 1.0f);

    // make the random numbers for x and y axis
    std::vector<float> seedsX;
    std::vector<float> seedsY;
    seedsX.resize(imageSize);
    seedsY.resize(imageSize);
    for (size_t i = 0; i < imageSize; ++i)
    {
        seedsX[i] = dist(rng);
        seedsY[i] = dist(rng);
    }

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = std::fmod(seedsX[x] + seedsY[y] + (x + y)*c_goldenRatioConjugate, 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Uniform : ((x + y) % repeatSize) / (repeatSize-1)
void Uniform (size_t imageSize, const char* fileName, size_t repeatSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = float((x + y) % repeatSize) / float(repeatSize - 1);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Uniform Jitter : ((x + y) % repeatSize) / (repeatSize-1) + rand(0, 1/repeatSize)
void UniformJitter (size_t imageSize, const char* fileName, size_t repeatSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0, float(1.0f / repeatSize));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = std::fmod(float((x + y) % repeatSize) / float(repeatSize - 1) + dist(rng), 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// White Noise : random pixel color
void WhiteNoise (size_t imageSize, const char* fileName)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0, 1.0f);

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = dist(rng);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Idea 7: Radial golden ratio.  Distance from center (in pixels) * golden ratio
void Idea7 (size_t imageSize, const char* fileName)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float distX = (float(x) - float(imageSize / 2));
            distX = distX*distX;

            float distY = (float(y) - float(imageSize / 2));
            distY = distY*distY;

            float distance = std::sqrt(distX + distY);

            float pixelValue = std::fmod(distance * c_goldenRatioConjugate, 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
void BlueNoise (size_t imageSize, const char* srcFileName, const char* destFileName)
{
    // report the name of the image we are working on
    printf("%s\n", destFileName);

    // load the blue noise source file
    SImageData srcImageData;
    ImageLoad(srcFileName, srcImageData);

    // copy just the portion we want
    SImageData imageData;
    ImageInit(imageData, imageSize, imageSize);
    ImageTile(srcImageData, imageData, srcImageData.m_width, srcImageData.m_height);

    // save the image
    ImageSave(imageData, destFileName);

    // do blue noise stipple test
    StippleTest(imageData, destFileName);

    // do blue noise dft
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(imageData, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, destFileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
void BlueNoiseChopTile (size_t imageSize, const char* srcFileName, const char* destFileName, size_t chopSize)
{
    // report the name of the image we are working on
    printf("%s\n", destFileName);

    // load the blue noise source file
    SImageData srcImageData;
    ImageLoad(srcFileName, srcImageData);

    // copy just the portion we want
    SImageData imageData;
    ImageInit(imageData, imageSize, imageSize);
    ImageTile(srcImageData, imageData, chopSize, chopSize);

    // save the image
    ImageSave(imageData, destFileName);

    // do blue noise stipple test
    StippleTest(imageData, destFileName);

    // do blue noise dft
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(imageData, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, destFileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// Interleaved Gradient Noise: From Jorge Jiminez.  http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare
void InterleavedGradientNoise (size_t imageSize, const char* fileName)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        float v = float(y);

        for (size_t x = 0; x < imageSize; ++x)
        {
            float u = float(x);

            float pixelValue = std::fmod(52.9829189f * std::fmod(0.06711056f*u + 0.00583715f*v, 1.0f), 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// UniformTest2 : pixel color = distance from uniform grid of dots. image normalized.
void UniformTest2 (size_t imageSize, const char* fileName, size_t gridSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // calculate distances
    std::vector<float> distances;
    distances.resize(imageSize * imageSize);
    float maxDist = 0.0f;
    for (size_t y = 0; y < imageSize; ++y)
    {
        for (size_t x = 0; x < imageSize; ++x)
        {
            float percentX = float(x) / float(imageSize);
            float percentY = float(y) / float(imageSize);

            float targetPercentX = std::floor((float(gridSize) * percentX) + 0.5f) / float(gridSize);
            float targetPercentY = std::floor((float(gridSize) * percentY) + 0.5f) / float(gridSize);

            float dx = percentX - targetPercentX;
            float dy = percentY - targetPercentY;

            float dist = std::sqrt(dx*dx + dy*dy);

            distances[y*imageSize + x] = dist;

            if (dist > maxDist)
                maxDist = dist;
        }
    }

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = distances[y * imageSize + x] / maxDist;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// UniformTest3 : just uniform dots
void UniformTest3 (size_t imageSize, const char* fileName, size_t gridSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = 1.0f;
            
            if ((x % gridSize) == 0 && (y%gridSize) == 0)
                pixelValue = 0.5f;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
template <size_t NUM_SAMPLES>
void GenerateHaltonPoints (std::array<std::array<float, 2>, NUM_SAMPLES>& samples, size_t basex, size_t basey)
{
    // calculate the sample points=
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
}

//======================================================================================
template <size_t NUM_SAMPLES>
void HaltonTestDots (size_t imageSize, const char* fileName)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // generate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    GenerateHaltonPoints(samples, 2, 3);

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            bool isASample = std::any_of(
                samples.begin(),
                samples.end(),
                [&] (std::array<float, 2>& sample) -> bool
                {
                    size_t pixelX = size_t(sample[0] * imageSize);
                    size_t pixelY = size_t(sample[1] * imageSize);
                    return pixelX == x && pixelY == y;
                }
            );

            float pixelValue = isASample ? 0.5f : 1.0f;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
template <size_t NUM_SAMPLES>
void HaltonTestDistance (size_t imageSize, const char* fileName)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // generate the sample points
    std::array<std::array<float, 2>, NUM_SAMPLES> samples;
    GenerateHaltonPoints(samples, 2, 3);

    // calculate distances
    std::vector<float> distances;
    distances.resize(imageSize * imageSize);
    float maxDist = 0.0f;
    for (size_t y = 0; y < imageSize; ++y)
    {
        for (size_t x = 0; x < imageSize; ++x)
        {
            float percentX = float(x) / float(imageSize);
            float percentY = float(y) / float(imageSize);

            float minDist = FLT_MAX;
            for (std::array<float, 2>& sample : samples)
            {
                float dx = sample[0] - percentX;
                float dy = sample[1] - percentY;
                float dist = std::sqrt(dx*dx + dy*dy);

                if (dist < minDist)
                    minDist = dist;
            }

            distances[y*imageSize + x] = minDist;

            if (minDist > maxDist)
                maxDist = minDist;
        }
    }

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = distances[y * imageSize + x] / maxDist;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
void GenerateJitterPoints (std::vector<std::array<float, 2>>& samples, size_t gridSize, size_t imageSize)
{
    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0, 1.0f / float(gridSize));

    size_t sampleIndex = 0;
    samples.resize(gridSize * gridSize);
    for (size_t y = 0; y < gridSize; ++y)
    {
        for (size_t x = 0; x < gridSize; ++x)
        {
            samples[sampleIndex][0] = dist(rng) + (float(x)) / (float(gridSize));
            samples[sampleIndex][1] = dist(rng) + (float(y)) / (float(gridSize));
            ++sampleIndex;
        }
    }
}

//======================================================================================
void JitterTestDots (size_t imageSize, const char* fileName, size_t gridSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // generate the sample points
    std::vector<std::array<float, 2>> samples;
    GenerateJitterPoints(samples, gridSize, imageSize);

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            bool isASample = std::any_of(
                samples.begin(),
                samples.end(),
                [&] (std::array<float, 2>& sample) -> bool
                {
                    size_t pixelX = size_t(sample[0] * imageSize);
                    size_t pixelY = size_t(sample[1] * imageSize);
                    return pixelX == x && pixelY == y;
                }
            );

            float pixelValue = isASample ? 0.5f : 1.0f;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
void JitterTestDistance (size_t imageSize, const char* fileName, size_t gridSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // generate the sample points
    std::vector<std::array<float, 2>> samples;
    GenerateJitterPoints(samples, gridSize, imageSize);

    // calculate distances
    std::vector<float> distances;
    distances.resize(imageSize * imageSize);
    float maxDist = 0.0f;
    for (size_t y = 0; y < imageSize; ++y)
    {
        for (size_t x = 0; x < imageSize; ++x)
        {
            float percentX = float(x) / float(imageSize);
            float percentY = float(y) / float(imageSize);

            float minDist = FLT_MAX;
            for (std::array<float, 2>& sample : samples)
            {
                float dx = sample[0] - percentX;
                float dy = sample[1] - percentY;
                float dist = std::sqrt(dx*dx + dy*dy);

                if (dist < minDist)
                    minDist = dist;
            }

            distances[y*imageSize + x] = minDist;

            if (minDist > maxDist)
                maxDist = minDist;
        }
    }

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            float pixelValue = distances[y * imageSize + x] / maxDist;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
void RecursiveGridTest (size_t imageSize, const char* fileName)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // make the pixels
    std::vector<bool> pixelWritten;
    pixelWritten.resize(imageSize*imageSize);
    std::fill(pixelWritten.begin(), pixelWritten.end(), false);
    float inverseNumPixels = 1.0f / (float(imageSize)*float(imageSize));
    size_t rank = 0;
    for (size_t gridSize = imageSize; gridSize > 0; gridSize /= 2)
    {
        for (size_t y = 0; y < imageSize; y += gridSize)
        {
            for (size_t x = 0; x < imageSize; x += gridSize)
            {
                if (pixelWritten[y*imageSize + x])
                    continue;

                float pixelValue = float(rank) * inverseNumPixels;
                uint8 pixelColor = uint8(pixelValue * 255.0f);

                SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch + x * 3];
                pixelWritten[y*imageSize + x] = true;
                pixel->Set(pixelColor, pixelColor, pixelColor);
                ++rank;
            }
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
size_t ReverseBits (size_t A, size_t numBits)
{
    size_t ret = 0;

    size_t targetMask = size_t(1) << (numBits - 1);
    size_t mask = 1;

    while (mask <= targetMask)
    {
        ret <<= 1;
        ret |= ((A & mask) != 0);

        mask <<= 1;
    }

    return ret;
}

//======================================================================================
size_t InterleaveBits (size_t A, size_t B, size_t numBits)
{
    size_t ret = 0;
    
    size_t mask = size_t(1) << (numBits-1);

    while (mask)
    {
        ret <<= 1;
        ret |= ((A & mask) != 0);

        ret <<= 1;
        ret |= ((B & mask) != 0);

        mask >>= 1;
    }

    return ret;
}

//======================================================================================
void ZOrderTest (size_t imageSize, const char* fileName, bool reverseBeforeInterleave)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // TODO: seems like it should be < not <=
    // calculate how many bits are needed in each coordinate
    size_t numBits = 0;
    while ((size_t(1) << numBits) <= imageSize)
        ++numBits;

    // make the pixels
    float inverseNumPixels = 1.0f / (float(imageSize)*float(imageSize));
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            size_t rank;
            
            if (reverseBeforeInterleave)
                rank = InterleaveBits(ReverseBits(x, numBits), ReverseBits(y, numBits), numBits);
            else
                rank = InterleaveBits(x, y, numBits);

            float pixelValue = float(rank) * inverseNumPixels;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
// NOTE: this function only works when gridSize is a power of 2
void BayerTest (size_t imageSize, const char* fileName, size_t gridSize)
{
    // report the name of the image we are working on
    printf("%s\n", fileName);

    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // calculate how many bits are needed in each coordinate
    size_t numBits = 0;
    while ((size_t(1) << numBits) < gridSize)
        ++numBits;

#if 0  //set to 1 to have it print out the bayer matrix
    for (size_t bayerY = 0; bayerY < gridSize; ++bayerY)
    {
        for (size_t bayerX = 0; bayerX < gridSize; ++bayerX)
        {
            printf("%zu ", ReverseBits(InterleaveBits(bayerX, bayerX^bayerY, numBits + 1), numBits * 2));
        }
        printf("\n");
    }
#endif

    // make the pixels
    float inverseNumPixels = 1.0f / (float(gridSize)*float(gridSize));
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            size_t bayerX = x % gridSize;
            size_t bayerY = y % gridSize;

            size_t rank = ReverseBits(InterleaveBits(bayerX, bayerX^bayerY, numBits + 1), numBits * 2);

            float pixelValue = float(rank) * inverseNumPixels;
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // do the stippling test
    StippleTest(image, fileName);

    // save the DFT amplitude of the image
    if (DO_DFT())
    {
        SImageDataComplex imageDFTData;
        DFTImage(image, imageDFTData);
        SImageData imageDFTMagnitude;
        GetMagnitudeData(imageDFTData, imageDFTMagnitude);
        char dftFileName[256];
        strcpy(dftFileName, fileName);
        strcat(dftFileName, ".mag.bmp");
        ImageSave(imageDFTMagnitude, dftFileName);
    }
    printf("\n");
}

//======================================================================================
int main(int argc, char** argv)
{
    s_logFile = fopen("Metrics.txt", "w+t");

    // load the image used for stippling tests
    ImageLoad("srcimages/stippleimage.bmp", s_stippleImage);

    // TODO: leave this in but need to call the function something else :P
    BlueNoise(256 / IMAGE_DOWNSIZE_FACTOR(), "srcimages/Diffusion.bmp", "outimages/Diffusion.bmp");

    /*
    WhiteNoise(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/WhiteNoise.bmp");

    BlueNoise(256 / IMAGE_DOWNSIZE_FACTOR(), "srcimages/BlueNoise_16.bmp", "outimages/BlueNoise_16.bmp");
    BlueNoise(256 / IMAGE_DOWNSIZE_FACTOR(), "srcimages/BlueNoise_32.bmp", "outimages/BlueNoise_32.bmp");
    BlueNoise(256 / IMAGE_DOWNSIZE_FACTOR(), "srcimages/BlueNoise_64.bmp", "outimages/BlueNoise_64.bmp");
    BlueNoise(256 / IMAGE_DOWNSIZE_FACTOR(), "srcimages/BlueNoise_128.bmp", "outimages/BlueNoise_128.bmp");
    BlueNoise(256 / IMAGE_DOWNSIZE_FACTOR(), "srcimages/BlueNoise_256.bmp", "outimages/BlueNoise_256.bmp");

    BlueNoiseChopTile(256 / IMAGE_DOWNSIZE_FACTOR(), "srcimages/BlueNoise_256.bmp", "outimages/BlueNoise_ChopTile32.bmp", 32);

    InterleavedGradientNoise(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/InterleavedGradient.bmp");

    Idea1(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea1_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Idea1(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea1_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Idea1(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea1_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Idea1(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea1_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    Idea2(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea2_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Idea2(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea2_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Idea2(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea2_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Idea2(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea2_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    Idea3(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea3_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Idea3(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea3_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Idea3(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea3_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Idea3(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea3_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    Idea4(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea4_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Idea4(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea4_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Idea4(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea4_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Idea4(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea4_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    Idea5(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea5.bmp");
    Idea6(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea6.bmp");
    Idea7(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Idea7.bmp");

    Uniform(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Uniform_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Uniform(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Uniform_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Uniform(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Uniform_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Uniform(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Uniform_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    UniformJitter(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformJitter_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    UniformJitter(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformJitter_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    UniformJitter(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformJitter_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    UniformJitter(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformJitter_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    UniformTest2(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformTest2_2.bmp", 2 / IMAGE_DOWNSIZE_FACTOR());
    UniformTest2(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformTest2_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    UniformTest2(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformTest2_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    UniformTest2(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformTest2_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());

    UniformTest3(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformTest3_2.bmp", 2 / IMAGE_DOWNSIZE_FACTOR());
    UniformTest3(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformTest3_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    UniformTest3(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformTest3_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    UniformTest3(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/UniformTest3_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());

    HaltonTestDots<100>(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/HaltonDots_100.bmp");
    HaltonTestDots<1000>(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/HaltonDots_1000.bmp");
    HaltonTestDots<10000>(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/HaltonDots_10000.bmp");
    HaltonTestDots<100000>(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/HaltonDots_100000.bmp");

    HaltonTestDistance<100>(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/HaltonDistance_100.bmp");
    HaltonTestDistance<1000>(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/HaltonDistance_1000.bmp");
    HaltonTestDistance<10000>(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/HaltonDistance_10000.bmp");
    HaltonTestDistance<100000>(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/HaltonDistance_100000.bmp");

    JitterTestDots(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/JitterDots_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    JitterTestDots(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/JitterDots_32.bmp", 32 / IMAGE_DOWNSIZE_FACTOR());
    JitterTestDots(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/JitterDots_64.bmp", 64 / IMAGE_DOWNSIZE_FACTOR());
    JitterTestDots(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/JitterDots_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());

    JitterTestDistance(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/JitterDistance_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    JitterTestDistance(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/JitterDistance_32.bmp", 32 / IMAGE_DOWNSIZE_FACTOR());
    JitterTestDistance(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/JitterDistance_64.bmp", 64 / IMAGE_DOWNSIZE_FACTOR());
    JitterTestDistance(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/JitterDistance_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR();

    RecursiveGridTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/RecursiveGrid.bmp");

    BayerTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Bayer_2.bmp", 2 / IMAGE_DOWNSIZE_FACTOR());
    BayerTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Bayer_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    BayerTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Bayer_8.bmp", 8 / IMAGE_DOWNSIZE_FACTOR());
    BayerTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Bayer_32.bmp", 32 / IMAGE_DOWNSIZE_FACTOR());
    BayerTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Bayer_64.bmp", 64 / IMAGE_DOWNSIZE_FACTOR());
    BayerTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Bayer_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    BayerTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/Bayer_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    */

    // TODO: temp!
    //ZOrderTest(16, "outimages/ZOrder.bmp", false);

    /*
    // TODO: This stuff still seems wrong, need to dig into it. Bayer size 2 seems right though... kinda confusing. maybe it is actually ok.
    ZOrderTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/ZOrder.bmp", false);
    ZOrderTest(256 / IMAGE_DOWNSIZE_FACTOR(), "outimages/ZOrderReversed.bmp", true);
    */



    fclose(s_logFile);

    return 0;
}
/*

TODO:

* try making red noise by inverting blue noise amplitudes and inverse DFT'ing. Just do this ahead of time and make the texture, put it next to blue noise.
 * show jay when you've done it (;

* uniform via recursive grids. for 4x4 you'd go (x%4==0)&&(y%4==0).  then %2, then %1. only add a sample that hasn't been filled yet. Keep rank of these pixels, that is greyscale.
 * Problem for notes: x axis definitely has preference.
 * Solution? maybe instead interleave bits and do z order
 ! done add this to notes when totally done

* uniform via Z order! interleave bits to get rank.
 * problem: favors lower numbers too much.
 * Solution? reverse it.
 ! done add this to notes when totally done

* reverse Z order
 * problem: I'm not sure yet, look at it and see.
 * solution: likely a good solution will be to xor x and y, and interleave that with x.  that gives bayer matrix! explain what bayer matrix gives us i think.


? try adding some small random noise to Bayer to try "jitter the regular sampling"? daniel from tweitter thought this up. (@nostalgiadriven)


* distance: blue noise
* dots: blue noise
* maybe get a source blue noise dots texture from other project for now. don't need to include mitchell here :P

* make blue noise source texture into 256x256?

* try l1 norm instead of distance to make diamonds? just one for fun?
 * a version of this for the one that's circles. (uniform distance)
 
? maybe try sinusoid of distance in blue noise to see if that gets rid of some frequencies or anything?

* bayer matrix
 * https://en.wikipedia.org/wiki/Ordered_dithering
 * M(i, j) = bit_reverse(bit_interleave(bitwise_xor(x, y), x)) / n ^ 2
 * make a 4x4 bayer matrix and compare vs the values on wikipedia

* triangle noise
* pixel arty stylish noise? like from the presentation that has triangle noise mentioned in it
* bayer matrix 



* TODO's in code
* convert all these images to png so you can use on the web? and maybe combine them to make it easier?
* remake all images before making blog post












Uniform : ((x + y) % repeatSize) / (repeatSize-1)
Uniform Jitter : ((x + y) % repeatSize) / (repeatSize-1) + rand(0, 1/repeatSize)
White Noise : random pixel color

Blue Noise: good stuff.  It tiles pretty well too. Chopping a larger blue noise texture down and tiling that does not work well though.

Idea 1: Make Shuffles for X. Add golden ratio on y.
 * Notes: visible structure
Idea 2: Make Shuffles for X. Add golden ratio on y, but modulus by shuffle size.
 * Notes: Seems worse than not modulusing. maybe not a big surprise.
Idea 3: Make Shuffles for X and Y.  add the numbers together to figure out how much to multiply golden ratio by.
 * Notes: there are a lot of frequencies...
Idea 4: Make Shuffles for X and Y.  multiply the numbers together to figure out how much to multiply golden ratio by.
 * Notes: The zeros suck and make streaks.  Since the numbers are < 1, they darken eachother too.
Idea 5: (x+y)*golden ratio
 * Notes: aweful
Idea 6: Random seed for each row and column. pixel = (seed for row + y * golden ratio) + (seed for column + x * golden ratio)
 * Notes: looks a lot like idea3 256 unsurprisingly
Idea 7: Radial golden ratio.  Distance from center (in pixels) * golden ratio
 * Notes: odd looking!

Uniform test2: color points based on their distance to grid points.
 * Notes: looks like an old style.  note 256 doesn't make sense as that is a black image since it's 256x256, so the distance is always 0.
Uniform test3: make a grid of half dark points. use that for stippling.
 * Notes: doesn't look bad! having the pixels be full dark instead of half dark makes them always be written, so doesn't make a decent image


HaltonTestDots : just dots arranged in the halton sequence
 * Notes: decent
HaltonTestDistance : pixels colored based on distance from halton
 * Notes: decent. when downsized the image looks sketched!

Jitter dots:
 * Notes: decent
Jitter Distance:
 * Notes: decent. I wonder how this compares to blue noise? looks similar


* BLOG
 * basic idea: can i make some 2d low discrepancy sequences (different than "sample points") using shuffling (format preserving encryption) and golden ratio?
  * thinking that a shuffle is like a low discrepancy sequence, and golden ratio is a low discrepancy sequence
 * start out with real shuffling and then replace with FPE if i see that regular shuffling works, since FPE approaches quality of real shuffling
 * document ideas as you try them.  Code and images and DFT (magnitude)
 * show blue noise and DFT of that as ideal goal to compare against
 * also white noise
 ! Jorge Jiminez has a good solution
  * ALAN: check presentation. it says something about a spiral pattern.
 * seems in some of the images are because i repeated a noise tile instead of calculating the noise over the whole domain (eg halton and interleaved gradient noise)

! will likely want to make some gifs, like showing the stippled bayer images. larger bayer matrices don't really add that much to the quality of the image

* free blue noise textures / blue noise info
 * http://momentsingraphics.de/?p=127

* stippling and blue noise
 * http://www.joesfer.com/?p=108

* bart wronski: Dithering part three – real world 2D quantization dithering
  * https://bartwronski.com/2016/10/30/dithering-part-three-real-world-2d-quantization-dithering/
  * he links to jorge's Interleaved gradient noise
   * http://www.iryoku.com/next-generation-post-processing-in-call-of-duty-advanced-warfare

* rendering "inside". great use of noise!
 * https://www.gdcvault.com/play/1023002/Low-Complexity-High-Fidelity-INSIDE
 * https://www.youtube.com/watch?v=RdN06E6Xn9E

* info including triangle noise
 * http://loopit.dk/banding_in_games.pdf
 * from https://twitter.com/pixelmager

* triangle noise shadertoys
 * https://www.shadertoy.com/view/4t2SDh
 * https://www.shadertoy.com/view/ltBSRG

* good read on a fairly decent blue noise algorithm, and also ordered dithering.
 * http://cv.ulichney.com/papers/1993-void-cluster.pdf

* Connection between samples and greyscale patterns is "ordered dithering"
 ? can you asnwer the questions about low discrepancy sequences etc?

* noise shaping, an audio thing.  Get quantization error (quantize and subtract from original?) and then shape the error to be whatever spectrum you want
 * http://wiki.hydrogenaud.io/index.php?title=Noise_shaping
 * idea: put noise where it will be noticed less.
 * apparently very similar results to blue noise when you shape it to be blue noise.

* 11 dithering algorithms: error diffusion, which is different than ordered dithering.
 * http://www.tannerhelland.com/4660/dithering-eleven-algorithms-source-code/
 

* bayer matrix:
 * Bayer matrix is where distance between numbers are maximized.
 * this page says it's optimal in that way: https://www.visgraf.impa.br/Courses/ip00/proj/Dithering1/ordered_dithering.html
 * It's supposed to have only high frequency components?
 ? i wonder how larger ones look vs blue noise?

*/