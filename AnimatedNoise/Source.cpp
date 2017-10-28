#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <stdint.h>
#include <vector>
#include <random>

typedef uint8_t uint8;

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
void ImageConvertToLuma (SImageData& image)
{
    for (size_t y = 0; y < image.m_height; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y * image.m_pitch];
        for (size_t x = 0; x < image.m_width; ++x)
        {
            float luma = float(pixel->R) * 0.3f + float(pixel->G) * 0.59f + float(pixel->B) * 0.11f;
            uint8 lumau8 = uint8(luma + 0.5f);
            pixel->R = lumau8;
            pixel->G = lumau8;
            pixel->B = lumau8;
            ++pixel;
        }
    }
}

//======================================================================================
void GenerateWhiteNoise (SImageData& image, size_t width, size_t height)
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
            uint8 value = dist(rng);
            pixel->R = value;
            pixel->G = value;
            pixel->B = value;
            ++pixel;
        }
    }
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
            float pixelValue = std::fmod(52.9829189f * std::fmod(0.06711056f*float(x + offsetX) + 0.00583715f*float(y + offsetY), 1.0f), 1.0f);
            uint8 value = uint8(pixelValue * 255.0f);
            pixel->R = value;
            pixel->G = value;
            pixel->B = value;
            ++pixel;
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
    // make noise
    SImageData noise;
    GenerateWhiteNoise(noise, ditherImage.m_width, ditherImage.m_height);

    // dither the image
    SImageData dither;
    DitherWithTexture(ditherImage, noise, dither);

    // save the noise and dither results
    ImageSave(noise, "out/still_noise_white.bmp");
    ImageSave(dither, "out/still_dither_white.bmp");
}

//======================================================================================
void DitherInterleavedGradientNoise (const SImageData& ditherImage)
{
    // make noise
    SImageData noise;
    GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    // dither the image
    SImageData dither;
    DitherWithTexture(ditherImage, noise, dither);

    // save the noise and dither results
    ImageSave(noise, "out/still_noise_ig.bmp");
    ImageSave(dither, "out/still_dither_ig.bmp");
}

//======================================================================================
void DitherBlueNoise (const SImageData& ditherImage, const SImageData& blueNoise)
{
    // dither the image
    SImageData dither;
    DitherWithTexture(ditherImage, blueNoise, dither);

    // save the noise and dither results
    ImageSave(blueNoise, "out/still_noise_bluenoise.bmp");
    ImageSave(dither, "out/still_dither_bluenoise.bmp");
}

//======================================================================================
int main (int argc, char** argv)
{
    // load the dither image
    SImageData ditherImage;
    if (!ImageLoad("src/ditherimage.bmp", ditherImage))
    {
        printf("Could not load src/ditherimage.bmp");
        return 0;
    }

    // load the blue noise images
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
    }

    // convert the dither image to luma (greyscale) and save it in the out directory
    ImageConvertToLuma(ditherImage);
    ImageSave(ditherImage, "out/ditherimage.bmp");
    
    // still image dither tests
    DitherWhiteNoise(ditherImage);
    DitherInterleavedGradientNoise(ditherImage);
    DitherBlueNoise(ditherImage, blueNoise[0]);



    return 0;
}

/*

TODO:
* animate the dithering.
 * 8 frames since we have 8 blue noise textures?
 * white noise is random each time
 * interleaved gradient noise uses a random offset each time?

 * should you also animate an incremental averaging one? or save that... i dunno.

* animated again using golden ratio
* csv of integration steps / make graphs

* TODOs in code

Blog:
* show a comparison of dithering the tree image using white noise, blue noise, interleaved gradient noise.
 * maybe put the noise pattern next to the dithered image as a single image
 * white noise is worst, blue noise is best, interleaved gradient noise is in the middle.

* what about animated noise?
 * have an extra dimension of time.
 * show animations of white noise, blue noise (get 8 from that place?), interleaved gradient noise (choose random offset for each frame?).
  * should there be slow and fast versions?
 * show the same with golden ratio added each frame.

* maybe also need to show some "integration over time" and graphs of how well it's integrating?
 * maybe a black / white sample per pixel each frame and incrementally average it to get integrated result.
 * should come out to even grey. (or should we do it with the image?)
 * show mean and standard deviation over time (csv)
 * animated gif as appropriate?

? do we want to show DFT frequency magnitude for any of these?

* R4 unit is the one that suggested adding golden ratio to blue noise.

Links:
* free blue noise textures sit
* interleaved gradient links

*/