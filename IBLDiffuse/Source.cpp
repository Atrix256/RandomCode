#define _CRT_SECURE_NO_WARNINGS
  
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <thread>
#include <atomic>

#define FORCE_SINGLETHREADED() 0

// ==================================================================================================================
const float c_pi = 3.14159265359f;

// ==================================================================================================================
typedef uint8_t uint8;

template <size_t N>
using TVector = std::array<float, N>;

typedef TVector<3> TVector3;
typedef TVector<2> TVector2;

// ==================================================================================================================
template <size_t N>
TVector<N> operator + (const TVector<N>& a, const TVector<N>& b)
{
    TVector<N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = a[i] + b[i];
    return result;
}

template <size_t N>
TVector<N> operator * (const TVector<N>& a, const TVector<N>& b)
{
    TVector<N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = a[i] * b[i];
    return result;
}

template <size_t N>
TVector<N> operator + (const TVector<N>& a, float b)
{
    TVector<N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = a[i] + b;
    return result;
}

template <size_t N>
TVector<N> operator * (const TVector<N>& a, float b)
{
    TVector<N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = a[i] * b;
    return result;
}

template <size_t N>
TVector<N> operator / (const TVector<N>& a, float b)
{
    TVector<N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = a[i] / b;
    return result;
}

template <size_t N>
float LenSquared (const TVector<N>& a)
{
    float length = 0.0f;
    for (size_t i = 0; i < N; ++i)
        length += a[i] * a[i];
    return length;
}

template <size_t N>
float Len (const TVector<N>& a)
{
    return std::sqrt(LenSquared(a));
}

template <size_t N>
void Normalize (const TVector<N>& a)
{
    a = a / Len(a);
}

template <size_t N>
size_t LargestMagnitudeComponent (const TVector<N>& a)
{
    size_t winningIndex = 0;
    for (size_t i = 1; i < N; ++i)
    {
        if (std::abs(a[i]) > std::abs(a[winningIndex]))
            winningIndex = i;
    }
    return winningIndex;
}

TVector<3> Cross (const TVector<3>& a, const TVector<3>& b)
{
    return
    {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

// ==================================================================================================================
template <typename T>
T Clamp (T value, T min, T max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

// ==================================================================================================================
 struct SImageData
{
    SImageData()
        : m_width(0)
        , m_height(0)
    { }
  
    size_t m_width;
    size_t m_height;
    size_t m_pitch;
    std::vector<uint8> m_pixels;
};

// ==================================================================================================================
SImageData g_srcImages[6];
SImageData g_destImages[6];

// ==================================================================================================================
void WaitForEnter ()
{
    printf("Press Enter to quit");
    fflush(stdin);
    getchar();
}

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
TVector3 SampleSourceCubeMap (const TVector3& direction)
{
    size_t largestComponent = LargestMagnitudeComponent(direction);
    bool negFace = direction[largestComponent] < 0.0f;
    TVector3 cubePos = direction / direction[largestComponent];
    if (negFace)
        cubePos = cubePos * -1.0f;

    size_t faceIndex = largestComponent;
    if (!negFace)
        faceIndex += 3;

    TVector2 uv = { 0.0f, 0.0f };
    switch (largestComponent)
    {
        case 0:
        {
            uv[0] = cubePos[2] * (negFace ? -1.0f : 1.0f);
            uv[1] = cubePos[1];
            break;
        }
        case 1:
        {
            uv[0] = cubePos[0];
            uv[1] = cubePos[2] * (negFace ? -1.0f : 1.0f);
            break;
        }
        case 2:
        {
            uv[0] = cubePos[0] * (negFace ? -1.0f : 1.0f);
            uv[1] = cubePos[1];
            break;
        }
    }
    uv = uv * 0.5f + 0.5f;

    size_t pixelX = (size_t)(float(g_srcImages[faceIndex].m_width-1) * uv[0]);
    pixelX = Clamp<size_t>(pixelX, 0, g_srcImages[faceIndex].m_width - 1);

    size_t pixelY = (size_t)(float(g_srcImages[faceIndex].m_height-1) * uv[1]);
    pixelY = Clamp<size_t>(pixelY, 0, g_srcImages[faceIndex].m_height - 1);

    size_t pixelOffset = pixelY * g_srcImages[faceIndex].m_pitch + pixelX * 3;

    return
    {
        float(g_srcImages[faceIndex].m_pixels[pixelOffset + 0]) / 255.0f,
        float(g_srcImages[faceIndex].m_pixels[pixelOffset + 1]) / 255.0f,
        float(g_srcImages[faceIndex].m_pixels[pixelOffset + 2]) / 255.0f,
    };
}

// ==================================================================================================================
TVector3 DiffuseIrradianceForNormal (const TVector3& normal)
{
    // adapted from https://learnopengl.com/#!PBR/IBL/Diffuse-irradiance
    TVector3 irradiance = { 0.0f, 0.0f, 0.0f };

    TVector3 up = { 0.0, 1.0, 0.0 };
    TVector3 right = Cross(up, normal);
    up = Cross(normal, right);

    float sampleDelta = 0.025f;
    size_t sampleCount = 0;
    for (float phi = 0.0f; phi < 2.0f * c_pi; phi += sampleDelta)
    {
        for (float theta = 0.0f; theta < 0.5f * c_pi; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            TVector3 tangentSample = { sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta) };
            // tangent space to world
            TVector3 sampleVec = right * tangentSample[0] + up * tangentSample[1] + normal * tangentSample[2];

            irradiance = irradiance + SampleSourceCubeMap(sampleVec) * cos(theta) * sin(theta);
            ++sampleCount;
        }
    }
    irradiance = irradiance * c_pi * (1.0f / float(sampleCount));

    return irradiance;
}

// ==================================================================================================================
void OnRowComplete ()
{
    // periodically report progress
    static std::atomic<size_t> s_progress(0);

    size_t progress = s_progress.fetch_add(1);

    size_t totalRows = 0;
    for (size_t i = 0; i < 6; ++i)
        totalRows += g_srcImages[i].m_height;

    size_t oldPercent = (size_t)(100.0f * float(progress-1) / float(totalRows));
    size_t newPercent = (size_t)(100.0f * float(progress) / float(totalRows));

    if (oldPercent != newPercent)
        printf("\r               \rProgress: %zu%%", newPercent);
}

// ==================================================================================================================
void ProcessFace (size_t faceIndex)
{
    TVector3 facePlane = { 0, 0, 0 };
    facePlane[faceIndex % 3] = (faceIndex / 3) ? 1.0f : -1.0f;
    TVector3 uAxis = { 0, 0, 0 };
    TVector3 vAxis = { 0, 0, 0 };
    switch (faceIndex % 3)
    {
        case 0:
        {
            uAxis[2] = (faceIndex / 3) ? 1.0f : -1.0f;
            vAxis[1] = 1.0f;
            break;
        }
        case 1:
        {
            uAxis[0] = 1.0f;
            vAxis[2] = (faceIndex / 3) ? 1.0f : -1.0f;
            break;
        }
        case 2:
        {
            uAxis[0] = (faceIndex / 3) ? 1.0f : -1.0f;
            vAxis[1] = 1.0f;
        }
    }

    SImageData &destData = g_destImages[faceIndex];
    for (size_t iy = 0; iy < destData.m_height; ++iy)
    {
        uint8* pixel = &destData.m_pixels[iy * destData.m_pitch];

        TVector2 uv;
        uv[1] = (float(iy) / float(destData.m_height - 1));
        for (size_t ix = 0; ix < destData.m_width; ++ix)
        {
            uv[0] = (float(ix) / float(destData.m_width - 1));

            // calculate the position of the pixel on the cube
            TVector3 normalDir =
                facePlane +
                uAxis * (uv[0] * 2.0f - 1.0f) +
                vAxis * (uv[1] * 2.0f - 1.0f);

            // get the diffuse irradiance for this direction
            TVector3 diffuseIrradiance = DiffuseIrradianceForNormal(normalDir);

            // store the resulting color
            pixel[0] = (uint8)Clamp(diffuseIrradiance[0] * 255.0f, 0.0f, 255.0f);
            pixel[1] = (uint8)Clamp(diffuseIrradiance[1] * 255.0f, 0.0f, 255.0f);
            pixel[2] = (uint8)Clamp(diffuseIrradiance[2] * 255.0f, 0.0f, 255.0f);
            pixel += 3;
        }

        // update progress
        OnRowComplete();
    }

    int ijkl = 0;
}

void ThreadFunc ()
{
    static std::atomic<size_t> s_faceIndex(0);

    size_t faceIndex = s_faceIndex.fetch_add(1);
    while (faceIndex < 6)
    {
        ProcessFace(faceIndex);
        faceIndex = s_faceIndex.fetch_add(1);
    }
}

// ==================================================================================================================
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

    // process each destination image, multithreadedly if we can / should.
    size_t numThreads = FORCE_SINGLETHREADED() ? 1 : std::thread::hardware_concurrency();
    if (numThreads > 1)
    {
        if (numThreads > 6)
            numThreads = 6;
        std::vector<std::thread> threads;
        threads.resize(numThreads);
        size_t faceIndex = 0;
        for (std::thread& t : threads)
            t = std::thread(ThreadFunc);
        for (std::thread& t : threads)
            t.join();
    }
    else
    {
        ThreadFunc();
    }

    // save the resulting images
    printf("\n");
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
* irradiance may be too bright.  Store in float arrays and normalize the result across all images.
* i guess the result can be small.  The tutorial says 32x32?
* profile if needed and find slow parts
* test 32 and 64 bit mode
* take source images from command line
* we may not need the normalize() function (or length / length squared then!)
* make sure code is cleaned up etc

Blog:
* Link to PBR / IBL tutorial: https://learnopengl.com/#!PBR/IBL/Diffuse-irradiance
* mention the thing about needing an HDR image format in reality.
* skyboxes from: http://www.custommapmakers.org/skyboxes.php
* and: https://opengameart.org/content/indoors-skyboxes

*/