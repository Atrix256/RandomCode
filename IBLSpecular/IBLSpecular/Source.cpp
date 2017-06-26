#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <thread>
#include <atomic>

#define SPLIT_SUM_SIZE() 256 // size in width and height of the split sum texture

#define BRDF_INTEGRATION_SAMPLE_COUNT() 1024

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
TVector3 Cross (const TVector3&a, const TVector3& b)
{
  return {
    a[1] * b[2] - a[2] * b[1],
    a[2] * b[0] - a[0] * b[2],
    a[0] * b[1] - a[1] * b[0]
  };
}

// ==================================================================================================================
float Dot (const TVector3& a, const TVector3& b)
{
    return
        a[0] * b[0] +
        a[1] * b[1] + 
        a[2] * b[2];
}

// ==================================================================================================================
TVector3 Normalize (const TVector3& v)
{
  float length = 0.0f;
  for (size_t i = 0; i < 3; ++i)
    length += v[i]*v[i];
  length = std::sqrt(length);

  TVector3 ret;
  for (size_t i = 0; i < 3; ++i)
    ret[i] = v[i] / length;

  return ret;
}

// ==================================================================================================================
TVector3 operator * (const TVector3& a, float b)
{
    return
    {
        a[0] * b,
        a[1] * b,
        a[2] * b
    };
}

// ==================================================================================================================
TVector3 operator + (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] + b[0],
        a[1] + b[1],
        a[2] + b[2]
    };
}

// ==================================================================================================================
TVector3 operator - (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] - b[0],
        a[1] - b[1],
        a[2] - b[2]
    };
}

// ==================================================================================================================
//                                     SBlockTimer
// ==================================================================================================================
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
float RadicalInverse_VdC (size_t bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(float(bits) * 2.3283064365386963e-10); // / 0x100000000
}
// ==================================================================================================================
TVector2 Hammersley (size_t i, size_t N)
{
    return TVector2{ float(i) / float(N), RadicalInverse_VdC(i) };
}

// ==================================================================================================================
TVector3 ImportanceSampleGGX (TVector2 Xi, TVector3 N, float roughness)
{
    float a = roughness*roughness;

    float phi = 2.0f * c_pi * Xi[0];
    float cosTheta = (float)std::sqrt((1.0 - Xi[1]) / (1.0 + (a*a - 1.0) * Xi[1]));
    float sinTheta = (float)std::sqrt(1.0 - cosTheta*cosTheta);

    // from spherical coordinates to cartesian coordinates
    TVector3 H;
    H[0] = cos(phi) * sinTheta;
    H[1] = sin(phi) * sinTheta;
    H[2] = cosTheta;

    // from tangent-space vector to world-space sample vector
    TVector3 up = abs(N[2]) < 0.999f ? TVector3{ 0.0f, 0.0f, 1.0f } : TVector3{ 1.0f, 0.0f, 0.0f };
    TVector3 tangent = Normalize(Cross(up, N));
    TVector3 bitangent = Cross(N, tangent);

    TVector3 sampleVec = tangent * H[0] + bitangent * H[1] + N * H[2];
    return Normalize(sampleVec);
}

// ==================================================================================================================
float GeometrySchlickGGX (float NdotV, float roughness)
{
    float a = roughness;
    float k = (a * a) / 2.0f;

    float nom   = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}

// ==================================================================================================================
float GeometrySmith (const TVector3& N, const TVector3& V, const TVector3& L, float roughness)
{
    float NdotV = max(Dot(N, V), 0.0f);
    float NdotL = max(Dot(N, L), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}  

// ==================================================================================================================
TVector2 IntegrateBRDF (float NdotV, float roughness)
{
    // from https://learnopengl.com/#!PBR/IBL/Specular-IBL
    TVector3 V;
    V[0] = std::sqrt(1.0f - NdotV*NdotV);
    V[1] = 0.0f;
    V[2] = NdotV;

    float A = 0.0f;
    float B = 0.0f;

    TVector3 N = { 0.0f, 0.0f, 1.0f };

    for (size_t i = 0; i < BRDF_INTEGRATION_SAMPLE_COUNT(); ++i)
    {
        TVector2 Xi = Hammersley(i, BRDF_INTEGRATION_SAMPLE_COUNT());
        TVector3 H = ImportanceSampleGGX(Xi, N, roughness);
        TVector3 L = Normalize(H * 2.0 * Dot(V, H) - V);

        float NdotL = max(L[2], 0.0f);
        float NdotH = max(H[2], 0.0f);
        float VdotH = max(Dot(V, H), 0.0f);

        if (NdotL > 0.0)
        {
            float G = GeometrySmith(N, V, L, roughness);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);

            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    A /= float(BRDF_INTEGRATION_SAMPLE_COUNT());
    B /= float(BRDF_INTEGRATION_SAMPLE_COUNT());
    return TVector2{ A, B };
}

// ==================================================================================================================
template <typename L>
void RunMultiThreaded (const char* label, const L& lambda, bool newline)
{
    SBlockTimer timer(label);
    size_t numThreads = FORCE_SINGLETHREADED() ? 1 : std::thread::hardware_concurrency();
    printf("Doing %s with %zu threads.\n", label, numThreads);
    if (numThreads > 1)
    {
        std::vector<std::thread> threads;
        threads.resize(numThreads);
        size_t faceIndex = 0;
        for (std::thread& t : threads)
            t = std::thread(lambda);
        for (std::thread& t : threads)
            t.join();
    }
    else
    {
        lambda();
    }
    if (newline)
        printf("\n");
}

// ==================================================================================================================
void OnRowComplete (size_t rowIndex, size_t numRows)
{
    // report progress
    int oldPercent = rowIndex > 0 ? (int)(100.0f * float(rowIndex - 1) / float(numRows)) : 0;
    int newPercent = (int)(100.0f * float(rowIndex) / float(numRows));
    if (oldPercent != newPercent)
        printf("\r               \rProgress: %i%%", newPercent);
}

// ==================================================================================================================
void GenerateSplitSumTextureThreadFunc (SImageData& splitSumTexture)
{
    static std::atomic<size_t> s_rowIndex(0);
    size_t rowIndex = s_rowIndex.fetch_add(1);
    while (rowIndex < SPLIT_SUM_SIZE())
    {
        // get the pixel at the start of this row
        uint8* pixel = &splitSumTexture.m_pixels[rowIndex * splitSumTexture.m_pitch];

        float roughness = float(rowIndex) / float(SPLIT_SUM_SIZE() - 1);

        for (size_t ix = 0; ix < SPLIT_SUM_SIZE(); ++ix)
        {
            float NdotV = float(ix) / float(SPLIT_SUM_SIZE() - 1);

            TVector2 integratedBRDF = IntegrateBRDF(NdotV, roughness);
            pixel[2] = uint8(integratedBRDF[0] * 255.0f);
            pixel[1] = uint8(integratedBRDF[1] * 255.0f);
            pixel[0] = 0;

            // move to the next pixel
            pixel += 3;
        }

        OnRowComplete(rowIndex, (size_t)SPLIT_SUM_SIZE());

        // get the next row to process
        rowIndex = s_rowIndex.fetch_add(1);
    }
}

// ==================================================================================================================
void GenerateSplitSumTexture ()
{
    SImageData splitSumTexture;
    splitSumTexture.m_width = SPLIT_SUM_SIZE();
    splitSumTexture.m_height = SPLIT_SUM_SIZE();
    splitSumTexture.m_pitch = 4 * ((splitSumTexture.m_width * 24 + 31) / 32);
    splitSumTexture.m_pixels.resize(splitSumTexture.m_height * splitSumTexture.m_pitch);

    RunMultiThreaded(
        "Generating Split Sum Texture",
        [&splitSumTexture] () { GenerateSplitSumTextureThreadFunc(splitSumTexture); },
        true
    );

    if (SaveImage("SplitSum.bmp", splitSumTexture))
        printf("Saved: SplitSum.bmp\n");
    else
        printf("Could not save image: SplitSum.bmp\n");
}

// ==================================================================================================================
int main (int argc, char **argcv)
{
    GenerateSplitSumTexture();

    system("pause");

    return 0;
}

/*

TODO:
? why does split sum have black at x = 0?
* pre-integrate cube maps for specular
* profile!

* TODO's in code

* #define's for image sizes and processing sizes
* SRGB correction (only for cube map, not split sum)

BLOG:
* Show split sum texture with fewer samples, as well as uniform sampling and random sampling?

*/