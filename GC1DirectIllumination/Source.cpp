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
#include "SPointLight.h"
#include "Utils.h"
#include "STimer.h"

//=================================================================================
//                                 SETTINGS
//=================================================================================

// image size
static const size_t c_imageWidth = 500;
static const size_t c_imageHeight = 500;

// threading toggle
static const bool c_forceSingleThreaded = false;

// camera
static const SVector c_cameraPos = { 0.0f, 0.0f, 0.0f };
static const SVector c_cameraRight = { 1.0f, 0.0f, 0.0f };
static const SVector c_cameraUp = { 0.0f, 1.0f, 0.0f };
static const SVector c_cameraFwd = { 0.0f, 0.0f, 1.0f };
static const float c_nearDist = 0.1f;
static const float c_cameraVerticalFOV = 90.0f * c_pi / 180.0f;

// Materials
auto c_materials = make_array(
    SMaterial(SVector(0.9f, 0.1f, 0.1f), SVector()),
    SMaterial(SVector(0.1f, 0.9f, 0.1f), SVector()),
    SMaterial(SVector(0.1f, 0.1f, 0.9f), SVector()),
    SMaterial(SVector(0.1f, 0.9f, 0.9f), SVector()),
    SMaterial(SVector(0.9f, 0.1f, 0.9f), SVector()),
    SMaterial(SVector(0.9f, 0.9f, 0.1f), SVector())
);

// Spheres
auto c_spheres = make_array(
    SSphere(SVector(0.0f, 0.0f, 10.0f), 5.0f, 5),
    SSphere(SVector(0.0f, 0.0f, 2.0f), 0.5f, 0),
    SSphere(SVector(-2.0f, 0.0f, 2.0f), 0.5f, 1),
    SSphere(SVector(2.0f, 0.0f, 2.0f), 0.5f, 2),
    SSphere(SVector(0.0f, 2.0f, 2.0f), 0.5f, 3),
    SSphere(SVector(0.0f,-2.0f, 2.0f), 0.5f, 4)
);

// Lights
auto c_pointLights = make_array(
    SPointLight(SVector(0.1f, 0.1f, 0.0f), SVector(5.0f, 1.0f, 1.0f)),
    SPointLight(SVector(-0.3f, 0.1f, 1.0f), SVector(1.0f, 1.0f, 5.0f))
);

//=================================================================================
static const size_t c_numPixels = c_imageWidth * c_imageHeight;
static const float c_aspectRatio = float(c_imageWidth) / float(c_imageHeight);
 
typedef uint8_t uint8;
typedef std::array<uint8, 3> BGR_U8;
typedef std::array<float, 3> RGB_F32;

//=================================================================================
bool Visible (const SVector& a, const SVector& dir, float length, TObjectID ignoreObjectID = c_invalidObjectID)
{
    SCollisionInfo collisionInfo;
    collisionInfo.m_maxCollisionTime = length;
    for (const SSphere& s : c_spheres)
    {
        if (RayIntersects(a, dir, s, collisionInfo, ignoreObjectID))
            return false;
    }

    return true;
}

//=================================================================================
SVector L_out (const SCollisionInfo& X, const SVector& dir)
{
    const SMaterial& material = c_materials[(size_t)X.m_materialID];

    // start with emissive lighting
    SVector ret = material.m_emissive;

    // add direction illumination from each light
    for (const SPointLight& pointLight : c_pointLights)
    {
        // get the normalized direction vector from surface to light, and the length
        SVector dirToLight = pointLight.m_position - X.m_intersectionPoint;
        float distToLight = Length(dirToLight);
        dirToLight /= distToLight;

        // if we can see from surface point to light, add the light in
        if (Visible(X.m_intersectionPoint, dirToLight, distToLight, X.m_objectID))
        {
            SVector biradiance = Biradiance(pointLight, X.m_intersectionPoint);
            // TODO: scattering stuff, instead of just using diffuse and the dot product thing / test (he abs' the dot product...)
            float dp = Dot(dirToLight, X.m_surfaceNormal);
            if (dp > 0.0f)
                ret += biradiance * material.m_diffuse * abs(dp);
        }
    }

    return ret;
}

//=================================================================================
SVector L_in (const SVector& X, const SVector& dir)
{
    // Test this ray against our geometry
    SCollisionInfo collisionInfo;
    for (const SSphere& s : c_spheres)
        RayIntersects(X, dir, s, collisionInfo);

    // if we didn't hit anything, return black / darkness
    if (collisionInfo.m_objectID == c_invalidObjectID)
        return SVector();

    // else, return the amount of light coming towards us from the object we hit
    return L_out(collisionInfo, -dir);
}

//=================================================================================
void RenderPixel (float u, float v, RGB_F32& pixel)
{
    // make (u,v) go from [-1,1] instead of [0,1]
    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    // find where the ray hits the near plane, and normalize that vector to get the ray direction.
    SVector rayStart = c_cameraPos + c_cameraFwd * c_nearDist;
    const float windowTop = tan(c_cameraVerticalFOV/2) * c_nearDist;
    rayStart += c_cameraUp * windowTop * v;
    rayStart += c_cameraRight * windowTop * u * c_aspectRatio;
    SVector rayDir = rayStart - c_cameraPos;
    Normalize(rayDir);

    // our pixel is the amount of light coming in to the position on our near plane from the ray direction.
    SVector out = L_in(rayStart, rayDir);

    // apply SRGB correction
    pixel[0] = powf(out.m_x, 1.0f / 2.2f);
    pixel[1] = powf(out.m_y, 1.0f / 2.2f);
    pixel[2] = powf(out.m_z, 1.0f / 2.2f);
}

//=================================================================================
void ThreadFunc(SImageData<c_imageWidth, c_imageHeight, RGB_F32>& image)
{
    static std::atomic<size_t> g_currentPixelIndex(-1);

    // render individual pixels across multiple threads until we run out of pixels to do
    size_t pixelIndex = ++g_currentPixelIndex;
    while (pixelIndex < image.NumPixels())
    {
        // get the current pixel's UV and actual memory location
        size_t x = pixelIndex % image.Width();
        size_t y = pixelIndex / image.Width();
        float xPercent = (float)x / (float)image.Width();
        float yPercent = (float)y / (float)image.Height();
        RGB_F32& pixel = image.m_pixels[y * image.Width() + x];

        // render the current pixel
        RenderPixel(xPercent, yPercent, pixel);

        // move to next pixel
        pixelIndex = ++g_currentPixelIndex;
    }
}
 
//=================================================================================
int main (int argc, char **argv)
{
    STimer Timer;

    // make an RGB f32 texture.
    // lower left of image is (0,0).
    SImageData<c_imageWidth, c_imageHeight, RGB_F32> image_RGB_F32;

    // spin up some threads to do work, and wait for them to be finished.
    size_t numThreads = c_forceSingleThreaded ? 1 : std::thread::hardware_concurrency();
    printf("Spinning up %i threads to make a %i x %i image\r\n", numThreads, c_imageWidth, c_imageHeight);
    std::vector<std::thread> threads;
    threads.resize(numThreads);
    for (std::thread& t : threads)
        t = std::thread(ThreadFunc, std::ref(image_RGB_F32));
    for (std::thread& t : threads)
        t.join();

    // Convert from RGB floating point to BGR u8
    SImageData<c_imageWidth, c_imageHeight, BGR_U8> image_BGR_U8;
    RGB_F32 *src = image_RGB_F32.m_pixels;
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

* need to make sure and handle PITCH for writing pixels correctly.  the BMP format requires it!
 * honestly i think we may need 2 image type classes... one for BMP output, and the other for internal float stuff

* Test that aspect ratio works correctly.  Try a rectangular image

* do todos in code

* how do you make it not have distortion at edges again? shadertoy demos address that
 * they make the near plane be a lot farther (like 6.0!)
 * i think the problem is actually how the rays are generated.  Try doing it as even steps in degrees, instead of how you have it right now.






NOTES IN CASE NOT COVERED IN FUTURE:
* dont forget to tone map to get from whatever floating point values back to 0..1
 * saturate towards white!
* bloom
* anti aliasing
* ambient lighting?

TODO LATER:
* make it so you can animate things over time at a specified frame rate.  write each frame to disk. combine with ffmpeg to make videos!
* maybe a blog post?
* make width / height be command line options?

*/