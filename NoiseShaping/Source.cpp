#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <stdint.h>
#include <vector>
#include <random>
#include <array>

typedef uint8_t uint8;
typedef int64_t int64;

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
template <size_t N>
void SaveFloatArrayAsBMP (const std::array<float, N>& data, const char* fileName, size_t imageSize)
{
    // init the image
    SImageData image;
    ImageInit(image, imageSize, imageSize);

    // write the data to the image
    const float* srcData = &data[0];
    for (size_t y = 0; y < image.m_height; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y*image.m_pitch];

        for (size_t x = 0; x < image.m_width; ++x)
        {
            uint8 value = uint8(255.0f * srcData[0]);

            pixel->Set(value, value, value);

            ++pixel;
            ++srcData;
        }
    }

    // save the image
    ImageSave(image, fileName);
}

//======================================================================================
int main (int argc, char ** argv)
{
    const size_t c_imageSize = 256;

    // seed the random number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // blue noise
    {
        std::array<float, c_imageSize*c_imageSize> noise;

        for (float& v : noise)
        {
            v = dist(rng);
        }

        SaveFloatArrayAsBMP(noise, "noise.bmp", c_imageSize);
        
        // TODO: iteratively blur, subtract, rescale, save
    }

    return 0;
}

/*

TODO:

* blue noise: 
 * generate white noise
 * blur it, subtract blur from white noise, make it be full range again.
 * to rehape it, make a struct of: value, x, y.  shuffle it. sort by value. replace value by percentage in list.
 * i think blur maybe needs to wrap around. maybe have blur work in integer coordinates?
 * show dft magnitude too!

* different blurs are different quality low pass filters.
 * box blur = low quality
 * gaussian blur = better.
 * best = sync.
 * maybe need to do a post on that next? or getting lost in the weeds...?
 * not sure if worth white, but could show box vs gaussian?

* if things are slow, can you thread it at all?

* do similar for red noise.

* animated gifs showing it evolve?

* maybe also animate blue noise w/ golden ratio, and mention that in this post? vs a flip book of blue noise?

* note: timothy lottes says to use sort that's more efficient for fixed sizes keys (radix sort), and fits it in 64 bits instead of a struct.

* note: you can't make blue noise by making white noise, doing DFT, modifying stuff, then doing IDFT. that is filtering it and is equivelant to what you are doing here.

? how to demo the quality of this noise?

* links
 * https://bartwronski.com/2016/10/30/dithering-part-two-golden-ratio-sequence-blue-noise-and-highpass-and-remap/
 * https://gpuopen.com/vdr-follow-up-fine-art-of-film-grain/
 * gaussian blur: https://blog.demofox.org/2015/08/19/gaussian-blur/

*/