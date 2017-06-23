#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <thread>
#include <atomic>

#define SPLIT_SUM_SIZE() 256 // size in width and height of the split sum texture

// TODO: see how adjusting this affects things
#define BRDF_INTEGRATION_SAMPLE_COUNT() 1024

// ==================================================================================================================
typedef uint8_t uint8;

template <size_t N>
using TVector = std::array<float, N>;

typedef TVector<3> TVector3;
typedef TVector<2> TVector2;

// ============================================================================================
//                                     SBlockTimer
// ============================================================================================
struct SBlockTimer
{
    SBlockTimer (const char* label)
    {
        m_start = std::chrono::high_resolution_clock::now();
        m_label = label;
    }

    ~SBlockTimer ()
    {
        std::chrono::duration<float> seconds = std::chrono::high_resolution_clock::now() - m_start;
        printf("%s took %0.2f seconds\n", m_label, seconds.count());
    }

    std::chrono::high_resolution_clock::time_point m_start;
    const char* m_label;
};

// ==================================================================================================================
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

 // ==================================================================================================================
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

// ==================================================================================================================
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
    infoHeader.biWidth = (long)image.m_width;
    infoHeader.biHeight = (long)image.m_height;
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

// ==================================================================================================================
TVector2 IntegrateBRDF (float ndotv, float roughness)
{
    // from https://learnopengl.com/#!PBR/IBL/Specular-IBL
    TVector3 V;
    V[0] = std::sqrt(1.0f - ndotv*ndotv);
    V[1] = 0.0f;
    V[2] = ndotv;

    float A = 0.0f;
    float B = 0.0f;

    TVector3 N = { 0.0f, 0.0f, 1.0f };

    for (size_t i = 0; i < BRDF_INTEGRATION_SAMPLE_COUNT(); ++i)
    {

    }

    return{ 0.0f, 1.0f };
}

// ==================================================================================================================
void GenerateSplitSumTexture ()
{
    SImageData splitSumTexture;

    splitSumTexture.m_width = SPLIT_SUM_SIZE();
    splitSumTexture.m_height = SPLIT_SUM_SIZE();
    splitSumTexture.m_pitch = 4 * ((splitSumTexture.m_width * 24 + 31) / 32);
    splitSumTexture.m_pixels.resize(splitSumTexture.m_height * splitSumTexture.m_pitch);

    for (size_t iy = 0; iy < SPLIT_SUM_SIZE(); ++iy)
    {
        // get the pixel at the start of this row
        uint8* pixel = &splitSumTexture.m_pixels[iy * splitSumTexture.m_pitch];

        float ndotv = float(iy) / float(SPLIT_SUM_SIZE() - 1);

        for (size_t ix = 0; ix < SPLIT_SUM_SIZE(); ++ix)
        {
            float roughness = float(ix) / float(SPLIT_SUM_SIZE() - 1);

            TVector2 integratedBRDF = IntegrateBRDF(ndotv, roughness);
            pixel[0] = uint8(integratedBRDF[0] * 255.0f);
            pixel[1] = uint8(integratedBRDF[1] * 255.0f);
            pixel[2] = 0;

            // move to the next pixel
            pixel += 3;
        }
    }

    if (SaveImage("SplitSum.bmp", splitSumTexture))
        printf("Saved: SplitSum.bmp\n");
    else
        printf("Could not save image: SplitSum.bmp\n");
}

// ==================================================================================================================
int main (int argc, char **argcv)
{
    GenerateSplitSumTexture();

    return 0;
}

/*

TODO:
* make splitsum texture
* pre-integrate cube maps for specular

* #define's for image sizes and processing sizes
* SRGB correction
* multithreaded with a define to make it single threaded

BLOG:

*/