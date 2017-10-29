#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <stdint.h>
#include <vector>
#include <random>

typedef uint8_t uint8;

const float c_goldenRatio = 1.61803398875f;

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

    // save the results
    SImageData combined;
    ImageCombine3(ditherImage, noise, dither, combined);
    ImageSave(combined, "out/still_whitenoise.bmp");
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

    // save the results
    SImageData combined;
    ImageCombine3(ditherImage, noise, dither, combined);
    ImageSave(combined, "out/still_ignoise.bmp");
}

//======================================================================================
void DitherBlueNoise (const SImageData& ditherImage, const SImageData& blueNoise)
{
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

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimatedIntegrated (const SImageData& ditherImage)
{
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

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimatedIntegrated (const SImageData& ditherImage, const SImageData blueNoise[8])
{
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

        // save the results
        SImageData combined;
        ImageCombine2(blueNoise[i], dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherWhiteNoiseAnimatedGoldenRatio (const SImageData& ditherImage)
{
    // make noise
    SImageData noise;
    GenerateWhiteNoise(noise, ditherImage.m_width, ditherImage.m_height);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgr_whitenoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        if (i > 0)
        {
            float add = float(i) * c_goldenRatio;

            for (size_t y = 0; y < noise.m_height; ++y)
            {
                SColor* pixel = (SColor*)&noise.m_pixels[y * noise.m_pitch];
                for (size_t x = 0; x < noise.m_height; ++x)
                {
                    float valueFloat = std::fmodf((float(pixel->R) / 255.0f) + add, 1.0f);
                    uint8 value = uint8(valueFloat * 255.0f);
                    pixel->R = value;
                    pixel->G = value;
                    pixel->B = value;
                    ++pixel;
                }
            }
        }

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
void DitherInterleavedGradientNoiseAnimatedGoldenRatio (const SImageData& ditherImage)
{
    // make noise
    SImageData noise;
    GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgr_ignoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        if (i > 0)
        {
            float add = float(i) * c_goldenRatio;

            for (size_t y = 0; y < noise.m_height; ++y)
            {
                SColor* pixel = (SColor*)&noise.m_pixels[y * noise.m_pitch];
                for (size_t x = 0; x < noise.m_height; ++x)
                {
                    float valueFloat = std::fmodf((float(pixel->R) / 255.0f) + add, 1.0f);
                    uint8 value = uint8(valueFloat * 255.0f);
                    pixel->R = value;
                    pixel->G = value;
                    pixel->B = value;
                    ++pixel;
                }
            }
        }

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
void DitherBlueNoiseAnimatedGoldenRatio (const SImageData& ditherImage, const SImageData& blueNoise)
{
    // copy the blue noise so we can modify it
    SImageData noise;
    ImageInit(noise, blueNoise.m_width, blueNoise.m_height);
    noise.m_pixels = blueNoise.m_pixels;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgr_bluenoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        if (i > 0)
        {
            float add = float(i) * c_goldenRatio;

            for (size_t y = 0; y < noise.m_height; ++y)
            {
                SColor* pixel = (SColor*)&noise.m_pixels[y * noise.m_pitch];
                for (size_t x = 0; x < noise.m_height; ++x)
                {
                    float valueFloat = std::fmodf((float(pixel->R) / 255.0f) + add, 1.0f);
                    uint8 value = uint8(valueFloat * 255.0f);
                    pixel->R = value;
                    pixel->G = value;
                    pixel->B = value;
                    ++pixel;
                }
            }
        }

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
void DitherWhiteNoiseAnimatedGoldenRatioIntegrated (const SImageData& ditherImage)
{
    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // make noise
    SImageData noise;
    GenerateWhiteNoise(noise, ditherImage.m_width, ditherImage.m_height);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgrint_whitenoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        if (i > 0)
        {
            float add = float(i) * c_goldenRatio;

            for (size_t y = 0; y < noise.m_height; ++y)
            {
                SColor* pixel = (SColor*)&noise.m_pixels[y * noise.m_pitch];
                for (size_t x = 0; x < noise.m_height; ++x)
                {
                    float valueFloat = std::fmodf((float(pixel->R) / 255.0f) + add, 1.0f);
                    uint8 value = uint8(valueFloat * 255.0f);
                    pixel->R = value;
                    pixel->G = value;
                    pixel->B = value;
                    ++pixel;
                }
            }
        }

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

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherInterleavedGradientNoiseAnimatedGoldenRatioIntegrated (const SImageData& ditherImage)
{
    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // make noise
    SImageData noise;
    GenerateInterleavedGradientNoise(noise, ditherImage.m_width, ditherImage.m_height, 0.0f, 0.0f);

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgrint_ignoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        if (i > 0)
        {
            float add = float(i) * c_goldenRatio;

            for (size_t y = 0; y < noise.m_height; ++y)
            {
                SColor* pixel = (SColor*)&noise.m_pixels[y * noise.m_pitch];
                for (size_t x = 0; x < noise.m_height; ++x)
                {
                    float valueFloat = std::fmodf((float(pixel->R) / 255.0f) + add, 1.0f);
                    uint8 value = uint8(valueFloat * 255.0f);
                    pixel->R = value;
                    pixel->G = value;
                    pixel->B = value;
                    ++pixel;
                }
            }
        }

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

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
}

//======================================================================================
void DitherBlueNoiseAnimatedGoldenRatioIntegrated (const SImageData& ditherImage, const SImageData& blueNoise)
{
    std::vector<float> integration;
    integration.resize(ditherImage.m_width * ditherImage.m_height);
    std::fill(integration.begin(), integration.end(), 0.0f);

    // copy the blue noise so we can modify it
    SImageData noise;
    ImageInit(noise, blueNoise.m_width, blueNoise.m_height);
    noise.m_pixels = blueNoise.m_pixels;

    // animate 8 frames
    for (size_t i = 0; i < 8; ++i)
    {
        char fileName[256];
        sprintf(fileName, "out/animgrint_bluenoise%zu.bmp", i);

        // add golden ratio to the noise after each frame
        if (i > 0)
        {
            float add = float(i) * c_goldenRatio;

            for (size_t y = 0; y < noise.m_height; ++y)
            {
                SColor* pixel = (SColor*)&noise.m_pixels[y * noise.m_pitch];
                for (size_t x = 0; x < noise.m_height; ++x)
                {
                    float valueFloat = std::fmodf((float(pixel->R) / 255.0f) + add, 1.0f);
                    uint8 value = uint8(valueFloat * 255.0f);
                    pixel->R = value;
                    pixel->G = value;
                    pixel->B = value;
                    ++pixel;
                }
            }
        }

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

        // save the results
        SImageData combined;
        ImageCombine2(noise, dither, combined);
        ImageSave(combined, fileName);
    }
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

    // Animated dither integration tests
    DitherWhiteNoiseAnimatedIntegrated(ditherImage);
    DitherInterleavedGradientNoiseAnimatedIntegrated(ditherImage);
    DitherBlueNoiseAnimatedIntegrated(ditherImage, blueNoise);

    // Golden ratio animated dither integration tests
    DitherWhiteNoiseAnimatedGoldenRatioIntegrated(ditherImage);
    DitherInterleavedGradientNoiseAnimatedGoldenRatioIntegrated(ditherImage);
    DitherBlueNoiseAnimatedGoldenRatioIntegrated(ditherImage, blueNoise[0]);

    return 0;
}

/*

TODO:
* make things use SImageData for each pixel.

* csv of integration steps / make graphs

* the golden ratio animation tests may benefit from showing the DFT, to see if adding GR changes anything?

* I think i have "ulimited" blue noise textures with those 8.
 * random pixel offset (it tiles well)
 * mirror x axis
 * mirror y axis
 * flip x and y
 * invert (1.0 - grey)
 ? ask SE if anyone can say how this would compare to having more textures?

* for integration, maybe show stills of higher sample counts, and also show variance etc graph.

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