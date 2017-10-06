#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <vector>
#include <stdint.h>
#include <random>
#include <algorithm>
#include <complex>
#include <thread>
#include <atomic>

typedef uint8_t uint8;

const float c_pi = 3.14159265359f;

const float c_goldenRatioConjugate = 1.61803398875f;

// settings to speed things up when iterating
#define IMAGE_DOWNSIZE_FACTOR() 1
#define DO_DFT() true

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
    destImage.m_pitch = srcImage.m_width * 3;
    if (destImage.m_pitch & 3)
    {
        destImage.m_pitch &= ~3;
        destImage.m_pitch += 4;
    }
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
    destImage.m_pitch = srcImage.m_width * 3;
    if (destImage.m_pitch & 3)
    {
        destImage.m_pitch &= ~3;
        destImage.m_pitch += 4;
    }
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
            float pixelValue = std::fmod(shuffleItemsX[x] + shuffleItemsY[y], 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

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
// Idea 3: Make Shuffles for X and Y.  multiply the numbers together to figure out how much to multiply golden ratio by.
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
            float pixelValue = std::fmod(shuffleItemsX[x] * shuffleItemsY[y], 1.0f);
            
            uint8 pixelColor = uint8(pixelValue * 255.0f);

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

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
int main (int argc, char** argv)
{
    Idea1(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea1_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Idea1(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea1_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Idea1(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea1_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Idea1(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea1_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    Idea2(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea2_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Idea2(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea2_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Idea2(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea2_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Idea2(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea2_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    Idea3(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea3_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Idea3(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea3_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Idea3(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea3_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Idea3(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea3_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    Idea4(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea4_4.bmp", 4 / IMAGE_DOWNSIZE_FACTOR());
    Idea4(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea4_16.bmp", 16 / IMAGE_DOWNSIZE_FACTOR());
    Idea4(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea4_128.bmp", 128 / IMAGE_DOWNSIZE_FACTOR());
    Idea4(256 / IMAGE_DOWNSIZE_FACTOR(), "images/Idea4_256.bmp", 256 / IMAGE_DOWNSIZE_FACTOR());

    return 0;
}
/*

Idea 1: Make Shuffles for X. Add golden ratio on y.
 * Notes: visible structure
Idea 2: Make Shuffles for X. Add golden ratio on y, but modulus by shuffle size.
 * Notes: Seems worse than not modulusing. maybe not a big surprise.
Idea 3: Make Shuffles for X and Y.  add the numbers together to figure out how much to multiply golden ratio by.
 * Notes: there are a lot of frequencies...
Idea 4: Make Shuffles for X and Y.  multiply the numbers together to figure out how much to multiply golden ratio by.
 * Notes: The zeros suck and make streaks.  Since the numbers are < 1, they darken eachother too.


Idea N: Make multiple rows and columns of shuffles.    add the numbers together to figure out how much to multiply golden ratio by.
 ? could we make multiple rows and columns of shuffles and have it use whichever is immediately to the left or below?

* maybe try gradient noise?



? am i multithreading DFT correctly? do i really not need any protection when writing to different pixels?

* BLOG
 * basic idea: can i make some 2d low discrepancy sequences (different than "sample points") using shuffling (format preserving encryption) and golden ratio?
 * start out with real shuffling and then replace with FPE if i see that regular shuffling works, since FPE approaches quality of real shuffling
 * document ideas as you try them.  Code and images and DFT (magnitude)
 * show blue noise and DFT of that as ideal goal to compare against
 * also white noise
 ? maybe also show using these images for dithering something?

*/