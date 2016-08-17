#pragma once

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <vector>

//=================================================================================
typedef uint8_t uint8;
typedef std::array<uint8, 3> BGR_U8;
typedef std::array<float, 3> RGB_F32;

//=================================================================================
struct SImageDataRGBF32
{
    SImageDataRGBF32 (size_t width, size_t height)
        : m_width(width)
        , m_height(height)
    {
        m_pixels.resize(m_width * m_height);
    }

    const size_t m_width;
    const size_t m_height;
    std::vector<RGB_F32> m_pixels;
};

//=================================================================================
struct SImageDataBGRU8
{
    SImageDataBGRU8 (size_t width, size_t height)
        : m_width(width)
        , m_height(height)
        , m_pitch(WidthToPitch(width))
    {
        m_pixels.resize(m_pitch * m_height);
    }

    static size_t WidthToPitch (size_t width)
    {
        size_t pitch = width * 3;
        if (pitch & 3)
        {
            pitch &= ~3;
            pitch += 4;
        }
        return pitch;
    }

    const size_t m_width;
    const size_t m_height;
    const size_t m_pitch;
    std::vector<uint8> m_pixels;
};

//=================================================================================
bool SaveImage (const char *fileName, const SImageDataBGRU8 &image)
{
    // open the file if we can
    FILE *file;
    file = fopen(fileName, "wb");
    if (!file)
        return false;
 
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
    infoHeader.biSizeImage = (DWORD)image.m_pixels.size();
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