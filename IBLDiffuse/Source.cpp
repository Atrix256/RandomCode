#define _CRT_SECURE_NO_WARNINGS
  
#include <stdio.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <functional>
#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <thread>
#include <atomic>
  
typedef uint8_t uint8;
 
struct SImageData
{
    SImageData()
        : m_width(0)
        , m_height(0)
    { }
  
    long m_width;
    long m_height;
    long m_pitch;
    std::vector<uint8> m_pixels;
};

SImageData g_srcImages[6];
SImageData g_destImages[6];
  
void WaitForEnter ()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}
  
bool LoadImage (const char *fileName, SImageData& imageData)
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
  
bool SaveImage (const char *fileName, const SImageData &image)
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
    infoHeader.biWidth = image.m_width;
    infoHeader.biHeight = image.m_height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = image.m_pixels.size();
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

void ProcessFace (size_t faceIndex)
{
    // TODO: do this
    SImageData &destData = g_destImages[faceIndex];
    float sum = 0.0f;
    for (size_t iy = 0; iy < destData.m_height; ++iy)
    {
        for (size_t ix = 0; ix < destData.m_width; ++ix)
        {
        }
    }

    int ijkl = 0;
}

void ThreadFunc ()
{
    static std::atomic<size_t> s_faceIndex;

    size_t faceIndex = s_faceIndex.fetch_add(1);
    while (faceIndex < 6)
    {
        ProcessFace(faceIndex);
        faceIndex = s_faceIndex.fetch_add(1);
    }
}
 
int main (int argc, char **argv)
{
    // TODO: take from command line
    const char* src = "Vasa\\Vasa";

    const char* srcPatterns[6] = {
        "%sLeft.bmp",
        "%sDown.bmp",
        "%sBack.bmp",
        "%sRight.bmp",
        "%sUp.bmp",
        "%sFront.bmp",
    };

    const char* destPatterns[6] = {
        "%sLeft_Diffuse.bmp",
        "%sDown_Diffuse.bmp",
        "%sBack_Diffuse.bmp",
        "%sRight_Diffuse.bmp",
        "%sUp_Diffuse.bmp",
        "%sFront_Diffuse.bmp",
    };

    // try and load the source images, while initializing the destination images
    for (size_t i = 0; i < 6; ++i)
    {
        // load source image if we can
        char srcFileName[256];
        sprintf(srcFileName, srcPatterns[i], src);
        if (LoadImage(srcFileName, g_srcImages[i]))
        {
            printf("Loaded: %s\n", srcFileName);
        }
        else
        {
            printf("Could not load image: %s\n", srcFileName);
            WaitForEnter();
            return 0;
        }

        // initialize destination image
        g_destImages[i].m_width = g_srcImages[i].m_width;
        g_destImages[i].m_height = g_srcImages[i].m_height;
        g_destImages[i].m_pitch = g_srcImages[i].m_pitch;
        g_destImages[i].m_pixels.resize(g_destImages[i].m_height * g_destImages[i].m_pitch);
    }

    // process each destination image, multithreaded.
    size_t numThreads = std::thread::hardware_concurrency();
    if (numThreads > 6)
        numThreads = 6;
    std::vector<std::thread> threads;
    threads.resize(numThreads);
    size_t faceIndex = 0;
    for (std::thread& t : threads)
        t = std::thread(ThreadFunc);
    for (std::thread& t : threads)
        t.join();

    // save the resulting images
    for (size_t i = 0; i < 6; ++i)
    {
        char destFileName[256];
        sprintf(destFileName, destPatterns[i], src);
        if (SaveImage(destFileName, g_destImages[i]))
        {
            printf("Saved: %s\n", destFileName);
        }
        else
        {
            printf("Could not save image: %s\n", destFileName);
            WaitForEnter();
            return 0;
        }
    }

    WaitForEnter();
    return 0;
}

/*

TODO:
* take in a skybox, do convolution against cos theta, output the resulting skybox
* should we make it multithreaded? spawn up a thread per face? it only needs read only data of the source images so maybe!


Blog:
* Link to PBR / IBL tutorial: https://learnopengl.com/#!PBR/IBL/Diffuse-irradiance
* mention the thing about needing an HDR image format in reality.
* skyboxes from: http://www.custommapmakers.org/skyboxes.php
* and: https://opengameart.org/content/indoors-skyboxes

*/