#include <atomic>
#include <vector>
#include <thread>
#include <random>
#include "TVector3.h"
#include "SMaterial.h"
#include "SAABB.h"
#include "SOBB.h"
#include "STriangle.h"
#include "SSphere.h"
#include "SQuad.h"
#include "SRayHitInfo.h"
#include "STimer.h"

#include <windows.h> // for bitmap headers

#define FORCE_SINGLE_THREAD() 0

// TODO: remove these and the associated functions / functionality
#define COSINE_WEIGHTED_HEMISPHERE_SAMPLES() 0
#define JITTER_AA() 0

#define RENDER_SCENE() 0
// Scenes:
//  0 = sphere on plane with wall, small light            (slow convergence)
//  1 = sphere on plane with wall, small light + blue sky (quick convergence)
//  2 = spheres in box with small bright light            (prettier scene, slow convergence)
//  3 = sphere in box with larger dimmer light            (prettier scene, quick convergence)
//  4 = furnace test

//=================================================================================
// User tweakable parameters - Scenes
//=================================================================================

// image size
const size_t c_imageWidth = 512;
const size_t c_imageHeight = 512;

// sampling parameters
const size_t c_samplesPerPixel = 100;
const size_t c_numBounces = 5;
const float c_rayBounceEpsilon = 0.001f;

#if RENDER_SCENE() == 0

// camera parameters - assumes no roll (z axis rotation) and assumes that the camera isn't looking straight up
const TVector3 c_cameraPos = {0.0f, 0.0f, -10.0f};
const TVector3 c_cameraLookAt = { 0.0f, 0.0f, 0.0f };
float c_nearPlaneDistance = 0.1f;
const float c_cameraVerticalFOV = 40.0f * c_pi / 180.0f;

// the scene
const std::vector<SSphere> c_spheres =
{
    //     Position         | Radius|       Emissive      |      Diffuse
    { {  4.0f,  4.0f, 6.0f }, 0.5f, { { 10.0f, 10.0f, 10.0f }, { 0.0f, 0.0f, 0.0f } } },   // light
    { {  0.0f,  0.0f, 4.0f }, 2.0f, { { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f } } },   // ball
};

const std::vector<STriangle> c_triangles = {};

const std::vector<SQuad> c_quads = {
    // floor
    SQuad({ -4.0f, -3.0f, -4.0f },{ -4.0f,  2.0f, -4.0f },{ -4.0f,  2.0f,  12.0f },{ -4.0f, -3.0f,  12.0f }, SMaterial({ 0.0f, 0.0f, 0.0f },{ 0.1f, 0.9f, 0.1f })),
    // green wall
    SQuad({ -15.0f, -2.0f, 15.0f },{ 15.0f, -2.0f, 15.0f },{ 15.0f, -2.0f, -15.0f },{ -15.0f, -2.0f, -15.0f }, SMaterial({ 0.0f, 0.0f, 0.0f },{ 0.9f, 0.1f, 0.1f })),
};

const std::vector<SAABB> c_aabbs = {};

const std::vector<SOBB> c_obbs = {};

const TVector3 c_rayMissColor = { 0.0f, 0.0f, 0.0f };

#elif RENDER_SCENE() == 1

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

const std::vector<STriangle> c_triangles = { };

const std::vector<SQuad> c_quads = {
    // floor
    SQuad({ -4.0f, -3.0f, -4.0f },{ -4.0f,  2.0f, -4.0f },{ -4.0f,  2.0f,  12.0f },{ -4.0f, -3.0f,  12.0f }, SMaterial({ 0.0f, 0.0f, 0.0f },{ 0.1f, 0.9f, 0.1f })),
    // green wall
    SQuad({ -15.0f, -2.0f, 15.0f },{ 15.0f, -2.0f, 15.0f },{ 15.0f, -2.0f, -15.0f },{ -15.0f, -2.0f, -15.0f }, SMaterial({ 0.0f, 0.0f, 0.0f },{ 0.9f, 0.1f, 0.1f })),
};

const std::vector<SAABB> c_aabbs = {};

const std::vector<SOBB> c_obbs = {};

const TVector3 c_rayMissColor = { 0.1f, 0.4f, 1.0f };

#elif RENDER_SCENE() == 2

// camera parameters - assumes no roll (z axis rotation) and assumes that the camera isn't looking straight up
const TVector3 c_cameraPos = { 278.0f, 273.0f, -800.0f };
const TVector3 c_cameraLookAt = { 278.0f, 273.0f, 0.0f };
float c_nearPlaneDistance = 0.1f;
const float c_cameraVerticalFOV = 40.0f * c_pi / 180.0f;

// the scene
// I slightly modified the cornell box from http://www.graphics.cornell.edu/online/box/data.html
const std::vector<SSphere> c_spheres = { };

const std::vector<STriangle> c_triangles = { };

const std::vector<SQuad> c_quads = {
    // floor
    SQuad({ 552.8f, 0.0f, 0.0f }, { 0.0f, 0.0f,   0.0f }, {   0.0f, 0.0f, 559.2f },{ 549.6f, 0.0f, 559.2f }, SMaterial({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f })),

    // Light
    SQuad({ 343.0f, 548.6f, 227.0f },{ 343.0f, 548.6f, 332.0f },{ 213.0f, 548.6f, 332.0f },{ 213.0f, 548.6f, 227.0f }, SMaterial({25.0f, 25.0f, 25.0f}, {0.78f, 0.78f, 0.78f})),

    // Cieling
    SQuad({ 556.0f, 548.8f,   0.0f },{ 556.0f, 548.8f, 559.2f },{ 0.0f, 548.8f, 559.2f },{ 0.0f, 548.8f,   0.0f }, SMaterial({0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f})),

    // back wall
    SQuad({549.6f,   0.0f, 559.2f},{  0.0f,   0.0f, 559.2f},{  0.0f, 548.8f, 559.2f},{556.0f, 548.8f, 559.2f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),

    // left wall
    SQuad({0.0f,   0.0f, 559.2f},{0.0f,   0.0f,   0.0f},{0.0f, 548.8f,   0.0f},{0.0f, 548.8f, 559.2f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f })),

    // right wall
    SQuad({552.8f,   0.0f,   0.0f},{549.6f,   0.0f, 559.2f},{556.0f, 548.8f, 559.2f},{556.0f, 548.8f,   0.0f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f })),
};

const std::vector<SAABB> c_aabbs = {};

const std::vector<SOBB> c_obbs = {
    SOBB(SAABB({ 185.5f, 82.5f, 169.0f },{ 82.5f, 82.5f, 82.5f },SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),{ 0.0f, 1.0f, 0.0f }, -17.0f * c_pi / 180.0f),
    SOBB(SAABB({ 368.5f, 165.0f, 351.25 },{ 82.5f, 165.0f, 82.5f }, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),{ 0.0f, 1.0f, 0.0f }, 107.0f * c_pi / 180.0f),
};

const TVector3 c_rayMissColor = { 0.0f, 0.0f, 0.0f };

#elif RENDER_SCENE() == 3

// camera parameters - assumes no roll (z axis rotation) and assumes that the camera isn't looking straight up
const TVector3 c_cameraPos = { 278.0f, 273.0f, -800.0f };
const TVector3 c_cameraLookAt = { 278.0f, 273.0f, 0.0f };
float c_nearPlaneDistance = 0.1f;
const float c_cameraVerticalFOV = 40.0f * c_pi / 180.0f;

// the scene
// I slightly modified the cornell box from http://www.graphics.cornell.edu/online/box/data.html
const std::vector<SSphere> c_spheres = { };

const std::vector<STriangle> c_triangles = { };

const std::vector<SQuad> c_quads = {
    // floor
    SQuad({ 552.8f, 0.0f, 0.0f }, { 0.0f, 0.0f,   0.0f }, {   0.0f, 0.0f, 559.2f },{ 549.6f, 0.0f, 559.2f }, SMaterial({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f })),

    // Cieling
    SQuad({ 556.0f, 548.8f,   0.0f },{ 556.0f, 548.8f, 559.2f },{ 0.0f, 548.8f, 559.2f },{ 0.0f, 548.8f,   0.0f }, SMaterial({ 1.0f, 1.0f, 1.0f }, { 0.78f, 0.78f, 0.78f })),

    // back wall
    SQuad({549.6f,   0.0f, 559.2f},{  0.0f,   0.0f, 559.2f},{  0.0f, 548.8f, 559.2f},{556.0f, 548.8f, 559.2f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),

    // left wall
    SQuad({0.0f,   0.0f, 559.2f},{0.0f,   0.0f,   0.0f},{0.0f, 548.8f,   0.0f},{0.0f, 548.8f, 559.2f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f })),

    // right wall
    SQuad({552.8f,   0.0f,   0.0f},{549.6f,   0.0f, 559.2f},{556.0f, 548.8f, 559.2f},{556.0f, 548.8f,   0.0f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f })),
};

const std::vector<SAABB> c_aabbs = { };

const std::vector<SOBB> c_obbs = {
    SOBB( SAABB({ 185.5f, 82.5f, 169.0f },{ 82.5f, 82.5f, 82.5f },SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),{ 0.0f, 1.0f, 0.0f }, -17.0f * c_pi / 180.0f),
    SOBB(SAABB({ 368.5f, 165.0f, 351.25}, {82.5f, 165.0f, 82.5f}, SMaterial({ 0.0f, 0.0f, 0.0f },{ 1.0f, 1.0f, 1.0f })),{ 0.0f, 1.0f, 0.0f }, 107.0f * c_pi / 180.0f),
};

const TVector3 c_rayMissColor = { 0.0f, 0.0f, 0.0f };

#elif RENDER_SCENE() == 4

// camera parameters - assumes no roll (z axis rotation) and assumes that the camera isn't looking straight up
const TVector3 c_cameraPos = { 0.0f, 0.0f, -10.0f };
const TVector3 c_cameraLookAt = { 0.0f, 0.0f, 0.0f };
float c_nearPlaneDistance = 0.1f;
const float c_cameraVerticalFOV = 40.0f * c_pi / 180.0f;

// the scene
const std::vector<SSphere> c_spheres =
{
    //     Position         | Radius|       Emissive      |      Diffuse
    { { 0.0f, 0.0f, 4.0f },  2.0f, { { 0.0f, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f } } },   // ball
};

const std::vector<STriangle> c_triangles = {};

const std::vector<SQuad> c_quads = {};

const std::vector<SAABB> c_aabbs = {};

const std::vector<SOBB> c_obbs = {};

const TVector3 c_rayMissColor = { 1.0f, 1.0f, 1.0f };

#endif

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


//=================================================================================
bool ClosestIntersection (const TVector3& rayPos, const TVector3& rayDir, SRayHitInfo& info)
{
    bool ret = false;
    for (const SSphere& s : c_spheres)
        ret |= RayIntersects(rayPos, rayDir, s, info);
    for (const STriangle& t : c_triangles)
        ret |= RayIntersects(rayPos, rayDir, t, info);
    for (const SQuad& q : c_quads)
        ret |= RayIntersects(rayPos, rayDir, q, info);
    for (const SAABB& a : c_aabbs)
        ret |= RayIntersects(rayPos, rayDir, a, info);
    for (const SOBB& o : c_obbs)
        ret |= RayIntersects(rayPos, rayDir, o, info);
    return ret;
}

//=================================================================================
TVector3 L_out (const SRayHitInfo& X, const TVector3& outDir, size_t bouncesLeft)
{
    // if no bounces left, return the ray miss color
    if (bouncesLeft == 0)
        return c_rayMissColor;

    // start with emissive lighting
    const SMaterial* material = X.m_material;

    TVector3 ret = material->m_emissive;

    // add in random recursive samples for global illumination
    {
#if COSINE_WEIGHTED_HEMISPHERE_SAMPLES()
        TVector3 newRayDir = CosineSampleHemisphere(X.m_surfaceNormal);
        SRayHitInfo info;
        if (ClosestIntersection(X.m_intersectionPoint + newRayDir * c_rayBounceEpsilon, newRayDir, info))
            ret += L_out(info, -newRayDir, bouncesLeft - 1) * material->m_diffuse;
        else
            ret += c_rayMissColor * material->m_diffuse;
#else
        TVector3 newRayDir = UniformSampleHemisphere(X.m_surfaceNormal);
        SRayHitInfo info;
        if (ClosestIntersection(X.m_intersectionPoint + newRayDir * c_rayBounceEpsilon, newRayDir, info))
            ret += Dot(newRayDir, X.m_surfaceNormal) * 2.0f * L_out(info, -newRayDir, bouncesLeft - 1) * material->m_diffuse;
        else
            ret += Dot(newRayDir, X.m_surfaceNormal) * 2.0f * c_rayMissColor * material->m_diffuse;
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
        return c_rayMissColor;

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
        float jitterX = JITTER_AA() ? RandomFloat() : 0.5f;
        float jitterY = JITTER_AA() ? RandomFloat() : 0.5f;

        // get the current pixel's UV coordinate and memory location
        size_t x = pixelIndex % c_imageWidth;
        size_t y = pixelIndex / c_imageWidth;
        float u = ((float)x + jitterX) / (float)c_imageWidth;
        float v = ((float)y + jitterY) / (float)c_imageHeight;
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
bool SaveImage (const char* fileName)
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

    // write the bitmap
    
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
    // get the filename from the command line, or use out.bmp if none supplied
    const char* fileName = argc < 2 ? "out.bmp" : argv[1];

    // report the params
    const size_t numThreads = FORCE_SINGLE_THREAD() ? 1 : std::thread::hardware_concurrency();
    printf("Rendering a %ix%i image with %i samples per pixel and %i ray bounces.\n", c_imageWidth, c_imageHeight, c_samplesPerPixel, c_numBounces);
    printf("Using %i threads.\n", numThreads);

    // allocate memory for our rendered image
    g_pixels.resize(c_numPixels);

    // time this block with an STimer
    {
        STimer timer;

        // if going multithreaded, spin up some threads to do rendering work, and wait for them to be done
        if (numThreads > 1) {
            auto start = std::chrono::high_resolution_clock::now();
            std::vector<std::thread> threads;
            threads.resize(numThreads);

            for (std::thread& t : threads)
                t = std::thread(ThreadFunc, std::ref(timer));

            for (std::thread& t : threads)
                t.join();
        }
        // else if single threaded, just call the rendering function from the main thread
        else {
            ThreadFunc(timer);
        }
    }

    // save the image
    if (!SaveImage(fileName))
        printf("Could not save image as %s.\n", fileName);
    else
        printf("Saved image as %s.\n", fileName);

    // all done
    system("pause");
    return 0;
}

/*

TODO:


* remove cosine weighted function from 1st blog post code, that is coming up next! (or, is coming up after AA)
 * and jitter AA, if we aren't keeping it in for the first blog post.  We probably are though.

----- BLOG -----
* show images with varying # of samples, and how long it took to render
 * maybe also vary # of bounces?
 * could show different axes of quality and how it affects runtime
  * image resolution
  * spp
  * max bounces

* could cache first hit of screen ray, since that will always be the same
 * not useful long term, as our anti aliasing will kill that speed up.
 ? should we do this and show differences in timing? or at least real quickly say how much faster something got?

* talk about how you can visualize things, like surface normals, by making them emissive values in the 0-1 range
 * or have a debugColor passed through that you can set to visualize max # of bounces and things
 * show debug visualization images

* make small (256x256?) gifs of each scene showing 1,10,100,1000,10000,100000 samples
 ? maybe: https://imgflip.com/images-to-gif -> water marked
 ? maybe : http://makeagif.com -> didn't work
 ? maybe: http://gifmaker.me/ -> seems to have worked!
 ? DAPSE code also has gif maker built in i think, maybe as command line!

* mention furnace test
* note how windows likes to cache images if you are viewing with the windows image viewer! delete file or zoom in / out.
 * takes the color out of images and other compression artifacts too!
* remake images at different levels
? include AA? it's probably super easy to explain so probably should.

* divide by pi or not
 * https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
 * more here: https://seblagarde.wordpress.com/2011/08/17/hello-world/
 * and wikipedia (which does): https://en.wikipedia.org/wiki/Path_tracing

* slow convergence discussion:
 * http://computergraphics.stackexchange.com/questions/3972/is-it-expected-that-a-naive-path-tracer-takes-many-many-samples-to-converge/3976#3976
 * the more different that samples are, the longer it will take to converge (could make an example of that if you care to?)

* random point on sphere
 * http://mathworld.wolfram.com/SpherePointPicking.html

* link to smallpt as a 99 lines of code path tracer - if code brevity helps understanding.

* perf test comparisons for primitives in cornell box scene
 * perf of Scene 3 512x512 with 5 bounces and 100 spp . "on my machine" don't need to give specs
 * 30 Triangles:    12.1 seconds
 * 15 Quads:         6.2 seconds -> makes sense. half the primitives, and quads are basically same cost as triangles due to how it tests
 * 5 quads, 2 obbs:  5.5 seconds

----- COSINE WEIGHTING -----

? why do we have to take the 2.0 off too, instead of just the cosine?
 * analyze distribution of samples to explain it in that post

----- Explicit light rays for larger lights -----
* https://gist.github.com/breakin/ed737ab44356b76ff21fc0c03e27e811
* also show how to do point lights!
* Can look better faster due to direct illumination. things in shadow / refraction / etc still take a while though.

----- Refraction -----
? does cosine law also apply to BSDF? if so, how?
* have to integrate over full sphere, not just hemisphere then!
 * http://computergraphics.stackexchange.com/questions/2482/choosing-reflection-or-refraction-in-path-tracing


*/