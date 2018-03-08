#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <cmath>
#include <vector>

// stb_image is an amazing header only image library (aka no linking, just include the headers!).  http://nothings.org/stb
#pragma warning( disable : 4996 ) 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#pragma warning( default : 4996 ) 

typedef uint8_t uint8;
typedef uint32_t uint32;

float sRGBToLinear(float value)
{
    if (value < 0.04045f)
        return value / 12.92f;
    else
        return std::powf(((value + 0.055f) / 1.055f), 2.4f);
}

float LinearTosRGB(float value)
{
    if (value < 0.0031308f)
        return value * 12.92f;
    else
        return std::powf(value, 1.0f / 2.4f) *  1.055f - 0.055f;
}

float GammaToLinear(float value, float gamma)
{
    return std::powf(value, gamma);
}

float LinearToGamma(float value, float gamma)
{
    return std::powf(value, 1.0f / gamma);
}

void sRGBTest(const char* fileName)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName, "w+t");
    if (!file)
    {
        printf("could not open %s for writing!\n", fileName);
        return;
    }

    fprintf(file, "\"Input sRGB u8\",\"Input sRGB f32\",\"Linear u8\",\"Linear f32\",\"Actual Linear f32\",\"Linear Error\",\"Output sRGB u8\",\"Output sRGB f32\",\"Output Error\"\n");
    for (uint32 i = 0; i < 256; ++i)
    {
        uint8 inputu8 = i;
        float inputf32 = float(i) / 255.0f;

        float actualLinearf32 = sRGBToLinear(inputf32);
        uint8 linearu8 = uint8(actualLinearf32*255.0f);
        float linearf32 = float(linearu8) / 255.0f;

        float outputf32 = LinearTosRGB(float(linearu8) / 255.0f);
        uint8 outputu8 = uint8(outputf32*255.0f);

        fprintf(file, "%u,%f,%u,%f,%f,%f,%u,%f,%f\n", inputu8, inputf32, linearu8, linearf32, actualLinearf32, std::fabsf(linearf32 - actualLinearf32), outputu8, outputf32, std::fabsf(outputf32 - inputf32));
    }

    fclose(file);
    printf("%s written.\n", fileName);
}

void GammaTest(const char* fileName, float gamma)
{
    FILE* file = nullptr;
    fopen_s(&file, fileName, "w+t");
    if (!file)
    {
        printf("could not open %s for writing!\n", fileName);
        return;
    }

    fprintf(file, "\"Input sRGB u8\",\"Input sRGB f32\",\"Linear u8\",\"Linear f32\",\"Actual Linear f32\",\"Linear Error\",\"Output sRGB u8\",\"Output sRGB f32\",\"Output Error\"\n");
    for (uint32 i = 0; i < 256; ++i)
    {
        uint8 inputu8 = i;
        float inputf32 = float(i) / 255.0f;

        float actualLinearf32 = GammaToLinear(inputf32, gamma);
        uint8 linearu8 = uint8(actualLinearf32*255.0f);
        float linearf32 = float(linearu8) / 255.0f;

        float outputf32 = LinearToGamma(float(linearu8) / 255.0f, gamma);
        uint8 outputu8 = uint8(outputf32*255.0f);

        fprintf(file, "%u,%f,%u,%f,%f,%f,%u,%f,%f\n", inputu8, inputf32, linearu8, linearf32, actualLinearf32, std::fabsf(linearf32 - actualLinearf32), outputu8, outputf32, std::fabsf(outputf32 - inputf32));
    }

    fclose(file);
    printf("%s written.\n", fileName);
}

void ImageTest (const char* fileName)
{
    // load the image if we can
    int channels, width, height;
    stbi_uc* pixels = stbi_load(fileName, &width, &height, &channels, 3);
    if (!pixels)
    {
        printf("could not load %s!\n", fileName);
        return;
    }

    // do a gamma 1.8 lossy conversion
    std::vector<uint8> imageData;
    imageData.resize(width*height * 3);
    for (size_t i = 0; i < imageData.size(); ++i)
    {
        uint8 linearu8 = uint8(std::powf(float(pixels[i]) / 255.0f, 1.8f)*255.0f);
        uint8 sRGBu8 = uint8(std::powf(float(linearu8) / 255.0f, 1.0f / 1.8f)*255.0f);
        imageData[i] = sRGBu8;
    }
    stbi_write_png("out_18.png", width, height, 3, &imageData[0], width*3);
    printf("out_18.png written\n");

    // write error image
    for (size_t i = 0; i < imageData.size(); ++i)
    {
        if (imageData[i] > pixels[i])
            imageData[i] = imageData[i] - pixels[i];
        else
            imageData[i] = pixels[i] - imageData[i];
    }
    stbi_write_png("out_18_error.png", width, height, 3, &imageData[0], width * 3);
    printf("out_18_error.png written\n");

    // do a gamma 2.2 lossy conversion
    for (size_t i = 0; i < imageData.size(); ++i)
    {
        uint8 linearu8 = uint8(std::powf(float(pixels[i]) / 255.0f, 2.2f)*255.0f);
        uint8 sRGBu8 = uint8(std::powf(float(linearu8) / 255.0f, 1.0f / 2.2f)*255.0f);
        imageData[i] = sRGBu8;
    }
    stbi_write_png("out_22.png", width, height, 3, &imageData[0], width * 3);
    printf("out_22.png written\n");

    // write error image
    for (size_t i = 0; i < imageData.size(); ++i)
    {
        if (imageData[i] > pixels[i])
            imageData[i] = imageData[i] - pixels[i];
        else
            imageData[i] = pixels[i] - imageData[i];
    }
    stbi_write_png("out_22_error.png", width, height, 3, &imageData[0], width * 3);
    printf("out_22_error.png written\n");

    // do a sRGB lossy conversion
    for (size_t i = 0; i < imageData.size(); ++i)
    {
        uint8 linearu8 = uint8(std::powf(float(pixels[i]) / 255.0f, 2.2f)*255.0f);
        uint8 sRGBu8 = uint8(std::powf(float(linearu8) / 255.0f, 1.0f / 2.2f)*255.0f);
        imageData[i] = sRGBu8;
    }
    stbi_write_png("out_sRGB.png", width, height, 3, &imageData[0], width * 3);
    printf("out_sRGB.png written\n");

    // write error image
    for (size_t i = 0; i < imageData.size(); ++i)
    {
        if (imageData[i] > pixels[i])
            imageData[i] = imageData[i] - pixels[i];
        else
            imageData[i] = pixels[i] - imageData[i];
    }
    stbi_write_png("out_sRGB_error.png", width, height, 3, &imageData[0], width * 3);
    printf("out_sRGB_error.png written\n");

    // free the source image
    stbi_image_free(pixels);
}

void MakeGradients()
{
    const size_t c_width = 256;
    const size_t c_height = 256;

    std::vector<uint8> pixels;
    pixels.resize(c_width * c_height * 3);

    uint8* pixel = &pixels[0];
    for (size_t y = 0; y < c_height; ++y)
    {
        for (size_t x = 0; x < c_width; ++x)
        {
            float percent = float(x) / float(c_width - 1);
            float red = percent;
            float green = 1.0f - percent;

            switch ((y * 4) / c_height)
            {
                case 0: break;
                case 1: red = LinearToGamma(red, 1.8f); green = LinearToGamma(green, 1.8f); break;
                case 2: red = LinearToGamma(red, 2.2f); green = LinearToGamma(green, 2.2f); break;
                case 3: red = LinearTosRGB(red); green = LinearTosRGB(green); break;
            }
            
            pixel[0] = uint8(red*255.0f);
            pixel[1] = uint8(green*255.0f);
            pixel[2] = 0;
            pixel += 3;
        }
    }

    stbi_write_png("out_gradients.png", c_width, c_height, 3, &pixels[0], c_width * 3);
    printf("out_gradients.png written\n");
}

int main(int argc, char** argv)
{
    MakeGradients();

    GammaTest("out_18.csv", 1.8f);
    GammaTest("out_22.csv", 2.2f);
    sRGBTest("out_sRGB.csv");

    ImageTest("scenery.png");

    system("pause");
    return 0;
}

/*

BLOG:

thanks to https://twitter.com/romainguy for suggesting red to green gradient!

* present the images and the error images
? make a gif flipping through the images, with a label on each frame?

* This is like the reverse of the toe in tone mapping right? it crushes the dark colors and is bad news.

sRGB formula
http://entropymine.com/imageworsener/srgbformula/

More info on sRGB in general:
Part 2 - https://bartwronski.com/2016/09/01/dynamic-range-and-evs/
Part 1 - https://bartwronski.com/2016/08/29/localized-tonemapping/

Timothy Lottes " Advanced Techniques and Optimization of HDR Color Pipelines"
https://gpuopen.com/gdc16-wrapup-presentations/

images made using stb_image -> http://nothings.org/stb

! do manually convert before calculating mips, but work in floating point to avoid the issue.
* can manually convert if you aren't going down to u8 again. that's where the problem comes in.

*/