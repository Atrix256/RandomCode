#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <thread>
#include <atomic>

// stb_image is an amazing header only image library (aka no linking, just include the headers!).  http://nothings.org/stb
#pragma warning( disable : 4996 ) 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#pragma warning( default : 4996 ) 

// Whether to make the splitsum texture or not (speeds it up if you don't need it)
#define MAKE_SPLITSUM() 1

// for debugging. Set to 1 to make it all run on the main thread.
#define FORCE_SINGLETHREADED() 0

// size in width and height of the split sum texture
#define SPLIT_SUM_SIZE() 256 

// number of samples per pixel for split sum texture
#define BRDF_INTEGRATION_SAMPLE_COUNT() 1024

// number of cube map mips to generate
#define MAX_MIP_LEVELS() 5

// Source images will be resized to this width and height in memory if they are larger than this
#define MAX_SOURCE_IMAGE_SIZE() 128

// If true, converts source images to linear before doing work. Output files are linear.
#define SOURCE_IS_SRGB() 1

// ==================================================================================================================
const float c_pi = 3.14159265359f;

// ==================================================================================================================
typedef uint8_t uint8;

template <size_t N>
using TVector = std::array<float, N>;

typedef TVector<3> TVector3;
typedef TVector<2> TVector2;

// ==================================================================================================================

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
template <size_t N>
inline TVector<N> operator * (const TVector<N>& a, float b)
{
    TVector<N> ret;
    for (size_t i = 0; i < N; ++i)
        ret[i] = a[i] * b;
    return ret;
}

// ==================================================================================================================
template <size_t N>
inline TVector<N> operator / (const TVector<N>& a, float b)
{
    TVector<N> ret;
    for (size_t i = 0; i < N; ++i)
        ret[i] = a[i] / b;
    return ret;
}

// ==================================================================================================================
template <size_t N>
inline TVector<N> operator + (const TVector<N>& a, float b)
{
    TVector<N> result;
    for (size_t i = 0; i < N; ++i)
        result[i] = a[i] + b;
    return result;
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
template <typename T>
inline T Clamp (T value, T min, T max)
{
    if (value < min)
        return min;
    else if (value > max)
        return max;
    else
        return value;
}

// ============================================================================================
// from http://www.rorydriscoll.com/2012/01/15/cubemap-texel-solid-angle/
inline float AreaElement(float x, float y)
{
    return std::atan2(x * y, std::sqrt(x * x + y * y + 1));
}

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

    size_t Pitch() const { return m_width * 3; }
  
    size_t m_width;
    size_t m_height;
    std::vector<float> m_pixels;
};

 // ==================================================================================================================
// t is a value that goes from 0 to 1 to interpolate in a C1 continuous way across uniformly sampled data points.
// when t is 0, this will return B.  When t is 1, this will return C.  Inbetween values will return an interpolation
// between B and C.  A and B are used to calculate slopes at the edges.
// More info at: https://blog.demofox.org/2015/08/15/resizing-images-with-bicubic-interpolation/
float CubicHermite (float A, float B, float C, float D, float t)
{
    float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
    float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;

    return a*t*t*t + b*t*t + c*t + d;
}

// ==================================================================================================================
const float* GetPixelClamped (const SImageData& image, int x, int y)
{
    x = Clamp<int>(x, 0, (int)image.m_width - 1);
    y = Clamp<int>(y, 0, (int)image.m_height - 1);
    return &image.m_pixels[(y * image.Pitch()) + x * 3];
}

// ==================================================================================================================
std::array<float, 3> SampleBicubic (const SImageData& image, float u, float v)
{
    // calculate coordinates -> also need to offset by half a pixel to keep image from shifting down and left half a pixel
    float x = (u * image.m_width) - 0.5f;
    int xint = int(x);
    float xfract = x - floor(x);

    float y = (v * image.m_height) - 0.5f;
    int yint = int(y);
    float yfract = y - floor(y);

    // 1st row
    auto p00 = GetPixelClamped(image, xint - 1, yint - 1);
    auto p10 = GetPixelClamped(image, xint + 0, yint - 1);
    auto p20 = GetPixelClamped(image, xint + 1, yint - 1);
    auto p30 = GetPixelClamped(image, xint + 2, yint - 1);

    // 2nd row
    auto p01 = GetPixelClamped(image, xint - 1, yint + 0);
    auto p11 = GetPixelClamped(image, xint + 0, yint + 0);
    auto p21 = GetPixelClamped(image, xint + 1, yint + 0);
    auto p31 = GetPixelClamped(image, xint + 2, yint + 0);

    // 3rd row
    auto p02 = GetPixelClamped(image, xint - 1, yint + 1);
    auto p12 = GetPixelClamped(image, xint + 0, yint + 1);
    auto p22 = GetPixelClamped(image, xint + 1, yint + 1);
    auto p32 = GetPixelClamped(image, xint + 2, yint + 1);

    // 4th row
    auto p03 = GetPixelClamped(image, xint - 1, yint + 2);
    auto p13 = GetPixelClamped(image, xint + 0, yint + 2);
    auto p23 = GetPixelClamped(image, xint + 1, yint + 2);
    auto p33 = GetPixelClamped(image, xint + 2, yint + 2);

    // interpolate bi-cubically!
    // Clamp the values since the curve can put the value below 0 or above 1
    std::array<float, 3> ret;
    for (int i = 0; i < 3; ++i)
    {
        float col0 = CubicHermite(p00[i], p10[i], p20[i], p30[i], xfract);
        float col1 = CubicHermite(p01[i], p11[i], p21[i], p31[i], xfract);
        float col2 = CubicHermite(p02[i], p12[i], p22[i], p32[i], xfract);
        float col3 = CubicHermite(p03[i], p13[i], p23[i], p33[i], xfract);
        float value = CubicHermite(col0, col1, col2, col3, yfract);
        ret[i] = Clamp(value, 0.0f, 1.0f);
    }
    return ret;
}

 // ==================================================================================================================
bool LoadImage (const char *fileName, SImageData& imageData)
{
    // load the image if we can
    int channels, width, height;
    stbi_uc* pixels = stbi_load(fileName, &width, &height, &channels, 3);
    if (!pixels)
        return false;

    // convert the pixels to float, and linear if they aren't already
    imageData.m_width = width;
    imageData.m_height = height;
    imageData.m_pixels.resize(imageData.Pitch()*imageData.m_height);

    for (size_t i = 0; i < imageData.m_pixels.size(); ++i)
    {
        imageData.m_pixels[i] = float(pixels[i]) / 255.0f;

        #if SOURCE_IS_SRGB()
        imageData.m_pixels[i] = sRGBToLinear(imageData.m_pixels[i]);
        #endif
    }

    // free the source image and return success
    stbi_image_free(pixels);
    return true;
}

// ==================================================================================================================
bool SaveImage (const char *fileName, const SImageData &image)
{
    std::vector<uint8> outPixels;
    outPixels.resize(image.m_pixels.size());
    for (size_t i = 0; i < image.m_pixels.size(); ++i)
    {
        float value = image.m_pixels[i];
        outPixels[i] = uint8(value * 255.0f);
    }

    return stbi_write_png(fileName, (int)image.m_width, (int)image.m_height, 3, &outPixels[0], (int)image.Pitch()) == 1;
}

// ==================================================================================================================
void DownsizeImage (SImageData& image, size_t imageSize)
{
    // repeatedly cut image in half (at max) until we reach the desired size.
    // done this way to avoid aliasing.
    while (image.m_width > imageSize)
    {
        // calculate new image size
        size_t newImageSize = image.m_width / 2;
        if (newImageSize < imageSize)
            newImageSize = imageSize;

        // allocate new image
        SImageData newImage;
        newImage.m_width = newImageSize;
        newImage.m_height = newImageSize;
        newImage.m_pixels.resize(newImage.m_height * newImage.Pitch());

        // sample pixels
        for (size_t iy = 0, iyc = newImage.m_height; iy < iyc; ++iy)
        {
            float percentY = float(iy) / float(iyc);

            float* destPixel = &newImage.m_pixels[iy * newImage.Pitch()];
            for (size_t ix = 0, ixc = newImage.m_width; ix < ixc; ++ix)
            {
                float percentX = float(ix) / float(ixc);

                std::array<float, 3> srcSample = SampleBicubic(image, percentX, percentY);
                destPixel[0] = srcSample[0];
                destPixel[1] = srcSample[1];
                destPixel[2] = srcSample[2];

                destPixel += 3;
            }
        }

        // set the image to the new image to possibly go through the loop again
        image = newImage;
    }
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
    float NdotV = std::max(Dot(N, V), 0.0f);
    float NdotL = std::max(Dot(N, L), 0.0f);
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

        float NdotL = std::max(L[2], 0.0f);
        float NdotH = std::max(H[2], 0.0f);
        float VdotH = std::max(Dot(V, H), 0.0f);

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
    printf("%s with %zu threads.\n", label, numThreads);
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

    const float c_halfAPixel = 0.5f / SPLIT_SUM_SIZE();

    while (rowIndex < SPLIT_SUM_SIZE())
    {
        // get the pixel at the start of this row
        float* pixel = &splitSumTexture.m_pixels[rowIndex * splitSumTexture.Pitch()];

        float roughness = float(rowIndex) / float(SPLIT_SUM_SIZE()) + c_halfAPixel;

        for (size_t ix = 0; ix < SPLIT_SUM_SIZE(); ++ix)
        {
            float NdotV = float(ix) / float(SPLIT_SUM_SIZE()) + c_halfAPixel;

            TVector2 integratedBRDF = IntegrateBRDF(NdotV, roughness);
            pixel[0] = integratedBRDF[0];
            pixel[1] = integratedBRDF[1];
            pixel[2] = 0;

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
    splitSumTexture.m_pixels.resize(splitSumTexture.m_height * splitSumTexture.Pitch());

    RunMultiThreaded(
        "Generating Split Sum Texture",
        [&splitSumTexture] () { GenerateSplitSumTextureThreadFunc(splitSumTexture); },
        true
    );

    if (SaveImage("SplitSum.png", splitSumTexture))
        printf("Saved: SplitSum.png\n\n");
    else
        printf("Could not save image: SplitSum.png\n\n");
}

// ==================================================================================================================
void DownsizeSourceThreadFunc (std::array<SImageData, 6>& srcImages)
{
    static std::atomic<size_t> s_imageIndex(0);
    size_t imageIndex = s_imageIndex.fetch_add(1);
    while (imageIndex < 6)
    {
        // downsize
        DownsizeImage(srcImages[imageIndex], MAX_SOURCE_IMAGE_SIZE());

        // get next image to process
        imageIndex = s_imageIndex.fetch_add(1);
    }
}

// ==================================================================================================================
void GetFaceBasis (size_t faceIndex, TVector3& facePlane, TVector3& uAxis, TVector3& vAxis)
{
    facePlane = { 0, 0, 0 };
    facePlane[faceIndex % 3] = (faceIndex / 3) ? 1.0f : -1.0f;
    uAxis = { 0, 0, 0 };
    vAxis = { 0, 0, 0 };
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

    if ((faceIndex % 3) == 2)
        facePlane[2] *= -1.0f;
}

// ==================================================================================================================
float DistributionGGX (TVector3 N, TVector3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = std::max(Dot(N, H), 0.0f);
    float NdotH2 = NdotH*NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = c_pi * denom * denom;

    return nom / denom;
}

// ==================================================================================================================
TVector3 SampleCubeMap (const std::array<SImageData, 6>& srcImages, const TVector3& dir)
{
    float maxAbs = abs(dir[0]);
    size_t maxComponent = 0;
    for (size_t i = 1; i < 3; ++i)
    {
        if (abs(dir[i]) >= maxAbs)
        {
            maxAbs = abs(dir[i]);
            maxComponent = i;
        }
    }

    size_t face = maxComponent;
    if (dir[maxComponent] > 0.0f)
        face += 3;

    // TODO: why is this code so special case. Is handedness different somewhere? A bug somewhere maybe?
    TVector2 uv;
    switch (maxComponent)
    {
        case 0:
        {
            uv[0] = dir[2];
            uv[1] = dir[1];

            if (dir[maxComponent] < 0.0f)
                uv[1] *= -1.0f;

            break;
        }
        case 1:
        {
            uv[0] = dir[0];
            uv[1] = dir[2];

            if (dir[maxComponent] < 0.0f)
                uv[0] *= -1.0f;

            break;
        }
        case 2:
        {
            face = (face + 3) % 6;
            uv[0] = -dir[0];
            uv[1] = dir[1];

            if (dir[maxComponent] < 0.0f)
                uv[1] *= -1.0f;

            break;
        }
    }

    uv = uv / dir[maxComponent];
    uv = uv * 0.5f + 0.5f;

    return SampleBicubic(srcImages[face], uv[0], uv[1]);
}

// ==================================================================================================================
TVector3 SpecularIrradianceForNormal (const std::array<SImageData, 6>& srcImages, const TVector3& normal, float roughness)
{
    // TODO: should we loop through all source pixels and use solid angle like we do with diffuse? i say yes, if we can!
    // TODO: clean this up
    // TODO: it had source image resolution as 512x512. what? maybe i need to process full sized images? i dunno.


    // make the simplyfying assumption that V equals R equals the normal 
    TVector3 R = normal;
    TVector3 V = R;

    const size_t SAMPLE_COUNT = 1024u;
    TVector3 prefilteredColor = { 0.0f, 0.0f, 0.0f };
    float totalWeight = 0.0f;

    for (size_t i = 0; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        TVector2 Xi = Hammersley(i, SAMPLE_COUNT);
        TVector3 H = ImportanceSampleGGX(Xi, normal, roughness);
        TVector3 L = Normalize(H * Dot(V, H) * 2.0f - V);

        float NdotL = std::max(Dot(normal, L), 0.0f);
        if (NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D = DistributionGGX(normal, H, roughness);
            float NdotH = std::max(Dot(normal, H), 0.0f);
            float HdotV = std::max(Dot(H, V), 0.0f);
            float pdf = D * NdotH / (4.0f * HdotV) + 0.0001f;

            float resolution = float(srcImages[0].m_width); // resolution of source cubemap (per face)
            float saTexel = 4.0f * c_pi / (6.0f * resolution * resolution);
            float saSample = 1.0f / (float(SAMPLE_COUNT) * pdf + 0.0001f);

            float mipLevel = roughness == 0.0f ? 0.0f : 0.5f * log2(saSample / saTexel);

            // TODO: sample the cube map at a specific mip level
            //prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;

            prefilteredColor = prefilteredColor + SampleCubeMap(srcImages, L) * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    return prefilteredColor;

#if 0
    // NOTE: this code is stale.It is from before pixels were float values and sRGB (instead of gamma 2.2) and before the output was kept linear
    // loop through every pixel in the source cube map and add that pixel's contribution to the diffuse irradiance
    float totalWeight = 0.0f;
    TVector3 irradiance = { 0.0f, 0.0f, 0.0f };
    for (size_t faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
const SImageData& src = srcImages[faceIndex];
TVector3 facePlane, uAxis, vAxis;
GetFaceBasis(faceIndex, facePlane, uAxis, vAxis);

float invResolution = 1.0f / src.m_width;

for (size_t iy = 0, iyc = src.m_height; iy < iyc; ++iy)
{
    TVector2 uv;
    uv[1] = (float(iy) / float(iyc - 1)) * 2.0f - 1.0f;

    const uint8* pixel = &src.m_pixels[iy * src.m_pitch];
    for (size_t ix = 0, ixc = src.m_width; ix < ixc; ++ix)
    {
        uv[0] = (float(ix) / float(ixc - 1)) * 2.0f - 1.0f;

        // only accept directions where dot product greater than 0
        TVector3 sampleDir =
            facePlane +
            uAxis * uv[0] +
            vAxis * uv[1];
        Normalize(sampleDir);
        float cosTheta = Dot(normal, sampleDir);
        if (cosTheta <= 0.0f)
            continue;

        // get the pixel color and move to the next pixel
        TVector3 pixelColor =
        {
            float(pixel[0]) / 255.0f,
            float(pixel[1]) / 255.0f,
            float(pixel[2]) / 255.0f,
        };
        pixel += 3;

        // calculate solid angle (size) of the pixel
        float x0 = uv[0] - invResolution;
        float y0 = uv[1] - invResolution;
        float x1 = uv[0] + invResolution;
        float y1 = uv[1] + invResolution;
        float solidAngle = AreaElement(x0, y0) - AreaElement(x0, y1) - AreaElement(x1, y0) + AreaElement(x1, y1);

        // add this pixel's contribution into the radiance
        irradiance = irradiance + pixelColor * solidAngle * cosTheta;

        /*
        // sample from the environment's mip level based on roughness/pdf
        float D = DistributionGGX(N, H, roughness);
        float NdotH = max(dot(N, H), 0.0);
        float HdotV = max(dot(H, V), 0.0);
        float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

        float resolution = 512.0; // resolution of source cubemap (per face)
        float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
        float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

        float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

        prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
        totalWeight += NdotL;
        */

        // keep track of the total weight so we can normalize later
        totalWeight += solidAngle;
    }
}
    }

    irradiance = irradiance * 4.0f / totalWeight;

    return irradiance;
#endif
}

// ==================================================================================================================
void ProcessRow(const std::array<SImageData, 6>& srcImages, std::array<SImageData, 6 * MAX_MIP_LEVELS()>& destImages, size_t rowIndex)
{
    // calculate which image we are on, while also making sure our row index is correct within that image.
    size_t imageIndex = 0;
    while (rowIndex >= destImages[imageIndex].m_height)
    {
        rowIndex -= destImages[imageIndex].m_height;
        ++imageIndex;
    }

    // calculate the face index and mip level from our image index
    size_t faceIndex = imageIndex % 6;
    size_t mipIndex = imageIndex / 6;

    // calculate roughness
    float roughness = (float)mipIndex / (float)(MAX_MIP_LEVELS() - 1);

    TVector3 facePlane, uAxis, vAxis;
    GetFaceBasis(faceIndex, facePlane, uAxis, vAxis);

    SImageData &destData = destImages[mipIndex * 6 + faceIndex];
    float* pixel = &destData.m_pixels[rowIndex * destData.Pitch()];

    TVector2 uv;
    uv[1] = ((float(rowIndex) + 0.5f) / float(destData.m_height));
    for (size_t ix = 0; ix < destData.m_width; ++ix)
    {
        uv[0] = ((float(ix) + 0.5f) / float(destData.m_width));

        // calculate the position of the pixel on the cube
        TVector3 normalDir =
            facePlane +
            uAxis * (uv[0] * 2.0f - 1.0f) +
            vAxis * (uv[1] * 2.0f - 1.0f);
        normalDir=Normalize(normalDir);

        // get the specular irradiance for this direction
        TVector3 specularIrradiance = SpecularIrradianceForNormal(srcImages, normalDir, roughness);

        // store the resulting color
        pixel[0] = Clamp(specularIrradiance[0], 0.0f, 1.0f);
        pixel[1] = Clamp(specularIrradiance[1], 0.0f, 1.0f);
        pixel[2] = Clamp(specularIrradiance[2], 0.0f, 1.0f);
        pixel += 3;
    }
}

// ==================================================================================================================
void ConvolutionThreadFunc (const std::array<SImageData, 6>& srcImages, std::array<SImageData, 6 * MAX_MIP_LEVELS()>& destImages)
{
    static std::atomic<size_t> s_rowIndex(0);

    size_t numRows = 0;
    for (const SImageData& image : destImages)
        numRows += image.m_height;

    size_t rowIndex = s_rowIndex.fetch_add(1);
    while (rowIndex < numRows)
    {
        ProcessRow(srcImages, destImages, rowIndex);
        OnRowComplete(rowIndex, numRows);
        rowIndex = s_rowIndex.fetch_add(1);
    }
}

// ==================================================================================================================
void GenerateCubeMap (const char* src)
{
    const char* srcPatterns[6] = {
        "%sLeft.bmp",
        "%sDown.bmp",
        "%sBack.bmp",
        "%sRight.bmp",
        "%sUp.bmp",
        "%sFront.bmp",
    };

    const char* destPatterns[6] = {
        "%s%iSpecularLeft.png",
        "%s%iSpecularDown.png",
        "%s%iSpecularBack.png",
        "%s%iSpecularRight.png",
        "%s%iSpecularUp.png",
        "%s%iSpecularFront.png",
    };

    // load the source images
    std::array<SImageData, 6> srcImages;
    for (size_t i = 0; i < 6; ++i)
    {
        char fileName[256];
        sprintf(fileName, srcPatterns[i], src);

        if (LoadImage(fileName, srcImages[i]))
        {
            printf("Loaded: %s (%zu x %zu)\n", fileName, srcImages[i].m_width, srcImages[i].m_height);

            if (srcImages[i].m_width != srcImages[i].m_height)
            {
                printf("image is not square!\n");
                return;
            }
        }
        else
        {
            printf("Could not load image: %s\n", fileName);
            return;
        }
    }

    // verify that the images are all the same size
    for (size_t i = 1; i < 6; ++i)
    {
        if (srcImages[i].m_width != srcImages[0].m_width || srcImages[i].m_height != srcImages[0].m_height)
        {
            printf("images are not all the same size!\n");
            return;
        }
    }

    // Resize source images in memory
    if (srcImages[0].m_width > MAX_SOURCE_IMAGE_SIZE())
    {
        printf("\nDownsizing source images in memory to %i x %i\n", MAX_SOURCE_IMAGE_SIZE(), MAX_SOURCE_IMAGE_SIZE());
        RunMultiThreaded(
            "Downsize source image",
            [&srcImages] () {DownsizeSourceThreadFunc(srcImages); },
            false
        );
    }
    printf("\n");

    // Allocate destination images
    std::array<SImageData, 6 * MAX_MIP_LEVELS()> destImages;
    size_t imageSize = srcImages[0].m_width;
    for (size_t mipIndex = 0; mipIndex < MAX_MIP_LEVELS(); ++mipIndex)
    {
        for (size_t faceIndex = 0; faceIndex < 6; ++faceIndex)
        {
            SImageData& destImage = destImages[mipIndex * 6 + faceIndex];

            destImage.m_width = imageSize;
            destImage.m_height = imageSize;
            destImage.m_pixels.resize(destImage.m_height * destImage.Pitch());
        }
        imageSize /= 2;
    }

    // Do the convolution
    RunMultiThreaded(
        "Doing convolution",
        [&srcImages, &destImages] () { ConvolutionThreadFunc(srcImages, destImages); },
        true
    );

    // save output images
    for (size_t mipIndex = 0; mipIndex < MAX_MIP_LEVELS(); ++mipIndex)
    {
        for (size_t faceIndex = 0; faceIndex < 6; ++faceIndex)
        {
            char destFileName[256];
            sprintf(destFileName, destPatterns[faceIndex], src, mipIndex);
            if (SaveImage(destFileName, destImages[mipIndex*6+faceIndex]))
            {
                printf("Saved: %s\n", destFileName);
            }
            else
            {
                printf("Could not save image: %s\n", destFileName);
                return;
            }
        }
    }
}

// ==================================================================================================================
int main (int argc, char **argcv)
{
    //const char* src = "Vasa\\Vasa";
    //const char* src = "ame_ash\\ashcanyon";
    //const char* src = "DallasW\\dallas";
    //const char* src = "MarriottMadisonWest\\Marriot";
    const char* src = "mnight\\mnight";

    #if MAKE_SPLITSUM()
        GenerateSplitSumTexture();
    #endif

    GenerateCubeMap(src);

    system("pause");

    return 0;
}

/*


TODO:
! it looks like the website starts with images that are 128x128

* cube map sampling function is awefully suspect.

* may need to offset uv by half a pixel when sampling cube maps. I think this may be why we sometimes get a single "wrong" row or column, with pixels from the other face.

? in diffuse project, should ConvolutionThreadFunc loop through dest images instead of source to get a row count?
 * same in ProcessRow()

? what size should images be before processing and after?
? why does split sum have black at x = 0? i think cause it needs to be plus half a pixel. in reality, we'd be doing center of pixel so wouldn't be at zero.
* pre-integrate cube maps for specular
* profile!

* TODO's in code

* #define's for image sizes and processing sizes
* SRGB correction (only for cube map, not split sum)

* the other one has "WaitForEnter" calls and this one uses pause (maybe the other does too?) standardize it!

BLOG:
* Show split sum texture with fewer samples, as well as uniform sampling and random sampling?

*/