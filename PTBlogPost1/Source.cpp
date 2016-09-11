#include <atomic>
#include <vector>
#include <thread>
#include <random>
#include "TVector3.h"
#include "SMaterial.h"
#include "STriangle.h"
#include "SSphere.h"
#include "SRayHitInfo.h"
#include "STimer.h"

#include <windows.h> // for bitmap headers

//=================================================================================
// User tweakable parameters
//=================================================================================

// image size
const size_t c_imageWidth = 512;
const size_t c_imageHeight = 512;

// sampling parameters
const size_t c_samplesPerPixel = 1000;
const size_t c_numBounces = 5;
const float c_rayBounceEpsilon = 0.001f;

// camera parameters - assumes no roll (z axis rotation) and assumes that the camera isn't looking straight up
const TVector3 c_cameraPos = {0.0f, 0.0f, -10.0f};
const TVector3 c_cameraLookAt = { 0.0f, 0.0f, 0.0f };
float c_nearPlaneDistance = 0.1f;
const float c_cameraVerticalFOV = 40.0f * c_pi / 180.0f;

// the scene
const std::vector<SSphere> c_spheres =
{
    //     Position         | Radius|       Emissive      |      Diffuse
    { {  4.0f,  4.0f, 6.0f }, 0.5f, { { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } } },   // light
    { {  0.0f,  0.0f, 4.0f }, 2.0f, { { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f } } },   // ball
};

const std::vector<STriangle> c_triangles =
{
    //                    A          |           B           |           C          |                Emissive       |      Diffuse
    // floor
    STriangle({ -15.0f, -2.0f, -15.0f }, { 15.0f, -2.0f, -15.0f }, { 15.0f, -2.0f, 15.0f }, SMaterial({ 0.0f, 0.0f, 0.0f }, { 0.9f, 0.1f, 0.1f })),
    STriangle({ -15.0f, -2.0f, -15.0f }, { 15.0f, -2.0f,  15.0f }, {-15.0f, -2.0f, 15.0f }, SMaterial({ 0.0f, 0.0f, 0.0f }, { 0.9f, 0.1f, 0.1f })),

    // green wall
    STriangle({  -4.0f, -2.0f,  12.0f }, { -4.0f,  2.0f,  12.0f }, { -4.0f,  2.0f, -4.0f }, SMaterial({ 0.0f, 0.0f, 0.0f }, { 0.1f, 0.9f, 0.1f })),
    STriangle({  -4.0f, -2.0f,  12.0f }, { -4.0f,  2.0f,  -4.0f }, { -4.0f, -2.0f, -4.0f }, SMaterial({ 0.0f, 0.0f, 0.0f }, { 0.1f, 0.9f, 0.1f })),
};

//=================================================================================
//=================================================================================
// Globals
std::atomic<size_t> g_currentPixelIndex(-1);
std::vector<TPixelRGBF32> g_pixels;

//=================================================================================
// Derived values
const size_t c_numPixels = c_imageWidth * c_imageHeight;
const float c_aspectRatio = float(c_imageWidth) / float(c_imageHeight);
const float c_cameraHorizFOV = c_cameraVerticalFOV * c_aspectRatio;
const float c_windowTop = tan(c_cameraVerticalFOV / 2.0f) * c_nearPlaneDistance;
const float c_windowRight = tan(c_cameraHorizFOV / 2.0f) * c_nearPlaneDistance;
const TVector3 c_cameraFwd = Normalize(c_cameraLookAt - c_cameraPos);
const TVector3 c_cameraRight = Cross({ 0.0f, 1.0f, 0.0f }, c_cameraFwd);
const TVector3 c_cameraUp = Cross(c_cameraFwd, c_cameraRight);

//=================================================================================
bool ClosestIntersection (const TVector3& rayPos, const TVector3& rayDir, SRayHitInfo& info)
{
    bool ret = false;
    for (const SSphere& s : c_spheres)
        ret |= RayIntersects(rayPos, rayDir, s, info);
    for (const STriangle& t : c_triangles)
        ret |= RayIntersects(rayPos, rayDir, t, info);
    return ret;
}

//=================================================================================
TVector3 L_out (const SRayHitInfo& X, const TVector3& outDir, size_t bouncesLeft)
{
    // if no bounces left, return black / darkness
    if (bouncesLeft == 0)
        return { 0.0f, 0.0f, 0.0f };

    // start with emissive lighting
    TVector3 ret = X.m_material->m_emissive;

    // add in random recursive samples for global illumination
    {
        const float pdf = 1.0f / c_pi;

#if COSINE_WEIGHTED_HEMISPHERE_SAMPLES()
        TVector3 newRayDir = CosineSampleHemisphere(X.m_surfaceNormal);
        SRayHitInfo info;
        if (ClosestIntersection(X.m_intersectionPoint + newRayDir * c_rayBounceEpsilon, newRayDir, info))
            ret += 2.0f * L_out(info, -newRayDir, bouncesLeft - 1) * X.m_material->m_diffuse / pdf;
#else
        TVector3 newRayDir = UniformSampleHemisphere(X.m_surfaceNormal);
        SRayHitInfo info;
        if (ClosestIntersection(X.m_intersectionPoint + newRayDir * c_rayBounceEpsilon, newRayDir, info))
            ret += Dot(newRayDir, X.m_surfaceNormal) * 2.0f * L_out(info, -newRayDir, bouncesLeft - 1) * X.m_material->m_diffuse / pdf;
#endif
    }

    return ret;
}

//=================================================================================
TPixelRGBF32 L_in (const TPixelRGBF32& rayPos, const TPixelRGBF32& rayDir)
{
    // if this ray doesn't hit anything, return black / darkness
    SRayHitInfo info;
    if (!ClosestIntersection(rayPos, rayDir, info))
        return { 0.0f, 0.0f, 0.0f };

    // else, return the amount of light coming towards us from the object we hit
    return L_out(info, -rayDir, c_numBounces);
}

//=================================================================================
void RenderPixel (float u, float v, TPixelRGBF32& pixel)
{
    // make (u,v) go from [-1,1] instead of [0,1]
    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    // find where the ray hits the near plane, and normalize that vector to get the ray direction.
    TVector3 rayStart = c_cameraPos + c_cameraFwd * c_nearPlaneDistance;
    rayStart += c_cameraRight * c_windowRight * u;
    rayStart += c_cameraUp * c_windowTop * v;
    TVector3 rayDir = Normalize(rayStart - c_cameraPos);

    // our pixel is the amount of light coming in to the position on our near plane from the ray direction
    pixel = L_in(rayStart, rayDir);
}

//=================================================================================
void ThreadFunc (STimer& timer)
{
    // each thread grabs a pixel at a time and renders it
    size_t pixelIndex = ++g_currentPixelIndex;
    bool firstThread = pixelIndex == 0;
    int lastPercent = -1;
    while (pixelIndex < c_numPixels)
    {
        // get the current pixel's UV coordinate and memory location
        size_t x = pixelIndex % c_imageWidth;
        size_t y = pixelIndex / c_imageWidth;
        float u = (float)x / (float)c_imageWidth;
        float v = (float)y / (float)c_imageHeight;
        TPixelRGBF32& pixel = g_pixels[pixelIndex];

        // render the pixel by taking multiple samples and incrementally averaging them
        for (size_t i = 0; i < c_samplesPerPixel; ++i)
        {
            TPixelRGBF32 sample;
            RenderPixel(u, v, sample);
            pixel += (sample - pixel) / float(i + 1.0f);
        }

        // move to next pixel
        pixelIndex = ++g_currentPixelIndex;

        // report our progress (from a single thread only)
        if (firstThread)
            timer.ReportProgress(pixelIndex, c_numPixels);
    }
}

//=================================================================================
bool SaveImage ()
{
    // allocate memory for our bitmap BGR U8 image
    std::vector<TPixelBGRU8> outPixels;
    outPixels.resize(c_numPixels);

    // convert from RGB F32 to BGR U8
    for (size_t i = 0; i < c_numPixels; ++i)
    {
        const TPixelRGBF32& src = g_pixels[i];
        TPixelBGRU8& dest = outPixels[i];

        // apply gamma correction
        TPixelRGBF32 correctedPixel;
        correctedPixel[0] = powf(src[0], 1.0f / 2.2f);
        correctedPixel[1] = powf(src[1], 1.0f / 2.2f);
        correctedPixel[2] = powf(src[2], 1.0f / 2.2f);

        // clamp and convert
        dest[0] = uint8(Clamp(correctedPixel[2] * 255.0f, 0.0f, 255.0f));
        dest[1] = uint8(Clamp(correctedPixel[1] * 255.0f, 0.0f, 255.0f));
        dest[2] = uint8(Clamp(correctedPixel[0] * 255.0f, 0.0f, 255.0f));
    }

    // write the bitmap to out.bmp
    
    // open the file if we can
    FILE *file;
    file = fopen("out.bmp", "wb");
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
    infoHeader.biWidth = (LONG)c_imageWidth;
    infoHeader.biHeight = (LONG)c_imageHeight;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = (DWORD)c_numPixels*3;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;
 
    header.bfSize = infoHeader.biSizeImage + header.bfOffBits;
 
    // write the data and close the file
    fwrite(&header, sizeof(header), 1, file);
    fwrite(&infoHeader, sizeof(infoHeader), 1, file);
    fwrite(&outPixels[0], infoHeader.biSizeImage, 1, file);
    fclose(file);
    return true;
}

//=================================================================================
int main (int argc, char**argv)
{
    // report the params
    const size_t numThreads = std::thread::hardware_concurrency();
    printf("Rendering a %ix%i image with %i samples per pixel and %i ray bounces.\n", c_imageWidth, c_imageHeight, c_samplesPerPixel, c_numBounces);
    printf("Using %i threads.\n", numThreads);

    // allocate memory for our rendered image
    g_pixels.resize(c_numPixels);

    // time this block with an STimer
    {
        STimer timer;

        // spin up some threads to do the rendering work
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<std::thread> threads;
        threads.resize(numThreads);
        for (std::thread& t : threads)
            t = std::thread(ThreadFunc, std::ref(timer));

        // wait for the threads to be done
        for (std::thread& t : threads)
            t.join();
        auto end = std::chrono::high_resolution_clock::now();
    }

    // save the image as out.bmp
    if (!SaveImage())
        printf("Could not save image.\n");
    else
        printf("Saved out.bmp.\n");

    // all done
    system("pause");
    return 0;
}

/*

TODO:

* do a couple different scenes
 * this current one to show how it takes a while to converge
 * something with small bright lights that take a while to converge
 * something in a box with large lights that takes a shorter time to converge
 * furnace test, to make sure it comes out ok

* use trig for UniformSampleHemisphere instead of looping

* profile to make sure there's no dumb perf issues

* remove cosine weighted function from 1st blog post code, that is coming up next!

----- BLOG -----
 * mention furnace test
 * note how windows likes to cache images if you are viewing with the windows image viewer! delete file or zoom in / out.

*/