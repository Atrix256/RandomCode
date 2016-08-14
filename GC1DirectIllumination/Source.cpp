#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <thread>
#include <atomic>

#include "SVector.h"
#include "SSphere.h"
#include "SMaterial.h"
#include "SImageData.h"

//=================================================================================

const float c_pi = 3.14159265359f;

//=================================================================================
//                                 SETTINGS
//=================================================================================

// image size
static const size_t c_imageWidth = 500;
static const size_t c_imageHeight = 500;

// threading
static const bool c_forceSingleThreaded = false;

// camera
static const SVector c_cameraPos = { 0.0f, 0.0f, 0.0f };
static const SVector c_cameraRight = { 1.0f, 0.0f, 0.0f };
static const SVector c_cameraUp = { 0.0f, 1.0f, 0.0f };
static const SVector c_cameraFwd = { 0.0f, 0.0f, 1.0f };
static const float c_nearDist = 0.01f;
static const float c_cameraVerticalFOV = 90.0f * c_pi / 180.0f;  // TODO: find a better vertical FOV? or maybe this is ok...

// Materials
SMaterial c_materials[] = {
    SMaterial(SVector(0.9f, 0.1f, 0.1f)),
    SMaterial(SVector(0.1f, 0.9f, 0.1f)),
    SMaterial(SVector(0.1f, 0.1f, 0.9f)),
    SMaterial(SVector(0.1f, 0.9f, 0.9f)),
    SMaterial(SVector(0.9f, 0.1f, 0.9f)),
    SMaterial(SVector(0.9f, 0.9f, 0.1f))
};
const size_t c_numMaterials = sizeof(c_materials) / sizeof(c_materials[0]);

// Spheres
SSphere c_spheres[] = {
    SSphere(SVector(0.0f, 0.0f, 10.0f), 5.0f, 5),
    SSphere(SVector(0.0f, 0.0f, 2.0f), 0.5f, 0),
    SSphere(SVector(-2.0f, 0.0f, 2.0f), 0.5f, 1),
    SSphere(SVector(2.0f, 0.0f, 2.0f), 0.5f, 2),
    SSphere(SVector(0.0f, 2.0f, 2.0f), 0.5f, 3),
    SSphere(SVector(0.0f,-2.0f, 2.0f), 0.5f, 4),
};
const size_t c_numSpheres = sizeof(c_spheres) / sizeof(c_spheres[0]);

//=================================================================================
static const size_t c_numPixels = c_imageWidth * c_imageHeight;
static const float c_aspectRatio = float(c_imageWidth) / float(c_imageHeight);
 
typedef uint8_t uint8;
typedef std::array<uint8, 3> BGR_U8;
typedef std::array<float, 3> RGB_F32;

//=================================================================================
// GLOBALS
//=================================================================================

// lower left of image is (0,0)
SImageData<c_imageWidth, c_imageHeight, RGB_F32> g_image_RGB_F32;

//=================================================================================
void RenderPixel (float u, float v, RGB_F32& pixel)
{
    // make (u,v) go from [-1,1] instead of [0,1]
    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    // find where the ray hits the near plane, and normalize that vector to get the ray direction.
    SVector rayDest = c_cameraPos + c_cameraFwd * c_nearDist;
    const float windowTop = tan(c_cameraVerticalFOV/2) * c_nearDist;
    rayDest += c_cameraUp * windowTop * v;
    rayDest += c_cameraRight * windowTop * u * c_aspectRatio;
    SVector rayDir = rayDest - c_cameraPos;
    Normalize(rayDir);
    
    // Test this ray against our geometry
    SVector diffuse(0.1f, 0.1f, 0.1f);
    SCollisionInfo collisionInfo;
    for (size_t i = 0; i < c_numSpheres; ++i)
        RayIntersects(c_cameraPos, rayDir, c_spheres[i], collisionInfo);
    if (collisionInfo.m_objectID != c_invalidObjectID)
        diffuse = c_materials[collisionInfo.m_materialID].m_diffuse;

    pixel[0] = diffuse.m_x;
    pixel[1] = diffuse.m_y;
    pixel[2] = diffuse.m_z;

    // apply SRGB correction
    pixel[0] = powf(pixel[0], 1.0f / 2.2f);
    pixel[1] = powf(pixel[1], 1.0f / 2.2f);
    pixel[2] = powf(pixel[2], 1.0f / 2.2f);
}

//=================================================================================
void ThreadFunc ()
{
    static std::atomic<size_t> g_currentPixelIndex(-1);

    // render individual pixels across multiple threads until we run out of pixels to do
    size_t pixelIndex = ++g_currentPixelIndex;
    while (pixelIndex < g_image_RGB_F32.NumPixels())
    {
        // get the current pixel's UV and actual memory location
        size_t x = pixelIndex % g_image_RGB_F32.Width();
        size_t y = pixelIndex / g_image_RGB_F32.Width();
        float xPercent = (float)x / (float)g_image_RGB_F32.Width();
        float yPercent = (float)y / (float)g_image_RGB_F32.Height();
        RGB_F32& pixel = g_image_RGB_F32.m_pixels[y * g_image_RGB_F32.Width() + x];

        // render the current pixel
        RenderPixel(xPercent, yPercent, pixel);

        // move to next pixel
        pixelIndex = ++g_currentPixelIndex;
    }
}
 
//=================================================================================
int main (int argc, char **argv)
{
    // spin up some threads to do work, and wait for them to be finished.
    size_t numThreads = c_forceSingleThreaded ? 1 : std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    threads.resize(numThreads);
    for (std::thread& t : threads)
        t = std::thread(ThreadFunc);
    for (std::thread& t : threads)
        t.join();

    // Convert from RGB floating point to BGR u8
    SImageData<c_imageWidth, c_imageHeight, BGR_U8> image_BGR_U8;
    RGB_F32 *src = g_image_RGB_F32.m_pixels;
    BGR_U8 *dest = image_BGR_U8.m_pixels;
    for (size_t i = 0; i < image_BGR_U8.NumPixels(); ++i)
    {
        (*dest)[0] = uint8((*src)[2] * 255.0f);
        (*dest)[1] = uint8((*src)[1] * 255.0f);
        (*dest)[2] = uint8((*src)[0] * 255.0f);
        ++src;
        ++dest;
    }

    // save the image
    SaveImage("out.bmp", image_BGR_U8);
    return 0;
}

/*

TODO:

* Set up renderer with Li and Lo functions like the text has it

* implement lambert and biradiance
 * Lambertian lighting is just no specular.
 * L dot N.  N is surface normal, L is direction to light.
 * Biradiance is a way of making lights call off over distance.

* make emissive work too since it's already in equation!

* need to make sure and handle PITCH for writing pixels correctly.  the BMP format requires it!

* make it print out how long it took to render images

* Test that aspect ratio works correctly.  Try a rectangular image

* do todos in code

* how do you make it not have distortion at edges again? shadertoy demos address that

* make a make_array function instead of using C arrays.  that will catch out of bounds access!

* make phantom types or enum classes for the IDs

* are you doing srgb correction correctly?

* get lighting to work like in the chapter



? write up blog post / put code up?
? what next?
 * more from other chapters?
 * tone mapping?
 * bloom?
 * anti aliasing?

*/