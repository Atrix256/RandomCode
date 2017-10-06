#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <vector>
#include <stdint.h>
#include <random>
#include <algorithm>
#include <complex>

typedef uint8_t uint8;

const float c_pi = 3.14159265359f;

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
std::complex<float> DFTPixel (const SImageData &srcImage, int K, int L)
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
  
    // calculate 2d dft (brute force, not using fast fourier transform)
    int lastPercent = -1;
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
        {
            // calculate DFT for that pixel / frequency
            destImage.m_pixels[y * destImage.m_width + x] = DFTPixel(srcImage, x, y);
        }
        int percent = int(100.0f * float(x) / float(srcImage.m_height));
        if (lastPercent != percent)
        {
            lastPercent = percent;
            printf("            \rDFT: %i%%", lastPercent);
        }
    }
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
void MakeLDS2D (size_t imageSize, const char* fileName, size_t shuffleSizeX)
{
    // initialize the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);
    ImageClear(image, SColor(255, 255, 255));

    // TODO: replace explicit shuffle with FPE

    // TODO: make shuffle shuffle floats that are a %. Work in floats until writing final pixel?

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());

    // make the shuffles for x axis
    std::vector<uint8> shuffleItemsX;
    shuffleItemsX.resize(imageSize);
    for (size_t i = 0; i < imageSize; ++i)
        shuffleItemsX[i] = uint8(i % (shuffleSizeX + 1));
    
    // TODO: shuffle each section individually!
    size_t shuffleStart = 0;
    while (shuffleStart < imageSize)
    {
        size_t shuffleEnd = min(shuffleStart + shuffleSizeX, imageSize);
        std::shuffle(shuffleItemsX.begin() + shuffleStart, shuffleItemsX.begin() + shuffleEnd, rng);
        shuffleStart += shuffleSizeX;
    }

    // make the pixels
    for (size_t y = 0; y < imageSize; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];

        for (size_t x = 0; x < imageSize; ++x)
        {
            // TODO: use y to add golden ratio, etc!
            // TOOD: do multiple tests with different settings, DFT them etc
            uint8 pixelColor = shuffleItemsX[x];

            pixel->Set(pixelColor, pixelColor, pixelColor);
            ++pixel;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // save the DFT amplitude of the image
    printf("Doing DFT...\n");
    SImageDataComplex imageDFTData;
    DFTImage(image, imageDFTData);
    SImageData imageDFTMagnitude;
    GetMagnitudeData(imageDFTData, imageDFTMagnitude);
    char dftFileName[256];
    strcpy(dftFileName, fileName);
    strcat(dftFileName, ".mag.bmp");
    ImageSave(imageDFTMagnitude, dftFileName);
}

//======================================================================================
int main (int argc, char** argv)
{
    MakeLDS2D(256, "images/LDS2D.bmp", 128);



    return 0;
}
/*

TODO: multithread DFT?

* shuffle x axis, then add y * golden ratio for y axis
* or better yet (?) shuffle x axis, and shuffle y axis, and add that shuffled value * golden ratio to y axis?
* could also try shuffling both x and y and add golden ratio * each?

* try the variations and DFT them perhaps?
 * maybe use them to dither something?

*/