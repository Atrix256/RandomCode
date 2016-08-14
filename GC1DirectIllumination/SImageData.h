#pragma once

#include <windows.h>  // for bitmap headers.  Sorry non windows people!

template <size_t WIDTH, size_t HEIGHT, typename PIXELTYPE>
struct SImageData
{
    SImageData()
    {
        m_pixels = new PIXELTYPE[NumPixels()];
    }

    ~SImageData()
    {
        delete[] m_pixels;
    }

    static size_t Width() { return WIDTH; }
    static size_t Height() { return HEIGHT; }
    static size_t NumPixels() { return WIDTH*HEIGHT; }

    PIXELTYPE* m_pixels;
};

//=================================================================================
template <size_t WIDTH, size_t HEIGHT, typename PIXELTYPE>
bool SaveImage (const char *fileName, const SImageData<WIDTH, HEIGHT, PIXELTYPE> &image)
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
    infoHeader.biWidth = image.Width();
    infoHeader.biHeight = image.Height();
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = image.NumPixels() * sizeof(PIXELTYPE);
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