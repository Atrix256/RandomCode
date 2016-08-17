#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <thread>
#include <atomic>

#include "SVector.h"
#include "SSphere.h"
#include "STriangle.h"
#include "SMaterial.h"
#include "SImageData.h"
#include "SPointLight.h"
#include "Utils.h"
#include "STimer.h"
#include "Random.h"

//=================================================================================
//                                 SETTINGS
//=================================================================================

// image size
static const size_t c_imageWidth = 1000;
static const size_t c_imageHeight = 1000;

// sampling
static const size_t c_samplesPerPixel = 32;
static const bool c_jitterSamples = true && (c_samplesPerPixel > 1);
static const size_t c_maxBounces = 5;

// threading toggle
static const bool c_forceSingleThreaded = false;

// camera - assumes no roll, and that (0,1,0) is up
static const SVector c_cameraPos = { 0.1f, 0.1f, -4.0f };
static const SVector c_cameraAt = { 0.0f, 0.0f, 0.0f };
static const float c_nearDist = 0.1f;
static const float c_cameraVerticalFOV = 60.0f * c_pi / 180.0f;

// Materials
auto c_materials = make_array(
    SMaterial(SVector(0.9f, 0.1f, 0.1f), SVector(), SVector(), true),                     // matte red
    SMaterial(SVector(0.1f, 0.9f, 0.1f), SVector(), SVector(), true),                     // matte green
    SMaterial(SVector(0.1f, 0.1f, 0.9f), SVector(), SVector(), true),                     // matte blue
    SMaterial(SVector(0.1f, 0.9f, 0.9f), SVector(), SVector(), true),                     // matte teal
    SMaterial(SVector(0.9f, 0.1f, 0.9f), SVector(), SVector(), true),                     // matte magenta
    SMaterial(SVector(0.9f, 0.9f, 0.1f), SVector(), SVector(), true),                     // matte yellow
    SMaterial(SVector(0.9f, 0.9f, 0.9f), SVector(), SVector(), true),                     // matte white
    SMaterial(SVector(0.01f, 0.01f, 0.01f), SVector(), SVector(1.0f, 1.0f, 1.0f), true),     // chrome
    SMaterial(SVector(), SVector(0.9f, 0.1f, 0.1f), SVector(), false),                    // emissive red
    SMaterial(SVector(), SVector(0.1f, 0.9f, 0.1f), SVector(), false),                    // emissive green
    SMaterial(SVector(), SVector(0.1f, 0.1f, 0.9f), SVector(), false),                    // emissive blue
    SMaterial(SVector(0.01f, 0.01f, 0.01f), SVector(), SVector(0.1f, 0.1f, 0.1f), true)   // walls
);

// Spheres
auto c_spheres = make_array(
    SSphere(SVector(-2.0f, 0.0f, 4.0f), 2.0f, 5),
    SSphere(SVector(0.0f, 0.0f, 2.0f), 0.5f, 0),
    SSphere(SVector(-2.0f, 0.0f, 2.0f), 0.5f, 1),
    SSphere(SVector(2.0f, 0.0f, 2.0f), 0.5f, 2),
    SSphere(SVector(0.0f, 2.0f, 2.0f), 0.5f, 3),
    SSphere(SVector(0.0f,-2.0f, 2.0f), 0.5f, 4),

    SSphere(SVector(0.5f, 0.1f, 0.0f), 0.03f, 8),     // red light
    SSphere(SVector(0.3f, -0.3f, 0.0f), 0.03f, 9),   // green light
    SSphere(SVector(-0.3f, 0.1f, -1.0f), 0.03f, 10)   // blue light
);

const float c_boxSize = 5.0f;

// Triangles
auto c_triangles = make_array(
    STriangle(SVector(1.5f, 1.0f, 1.25f), SVector(1.5f, 0.0f, 1.75f), SVector(0.5f, 0.0f, 2.25f), 5),
    STriangle(SVector(0.5f, 0.25f, 3.5f), SVector(1.5f, 0.25f, 3.0f), SVector(1.5f, 1.25f, 2.5f), 5),

    // box wall - in front
    STriangle(SVector(-c_boxSize, -c_boxSize,  c_boxSize), SVector( c_boxSize, -c_boxSize,  c_boxSize), SVector( c_boxSize,  c_boxSize,  c_boxSize), 11),
    STriangle(SVector(-c_boxSize, -c_boxSize,  c_boxSize), SVector(-c_boxSize,  c_boxSize,  c_boxSize), SVector( c_boxSize,  c_boxSize,  c_boxSize), 11),

    // box wall - left
    STriangle(SVector(-c_boxSize, -c_boxSize,  c_boxSize), SVector(-c_boxSize, -c_boxSize, -c_boxSize), SVector(-c_boxSize, c_boxSize, -c_boxSize), 11),
    STriangle(SVector(-c_boxSize, -c_boxSize,  c_boxSize), SVector(-c_boxSize,  c_boxSize,  c_boxSize), SVector(-c_boxSize, c_boxSize, -c_boxSize), 11),

    // box wall - right
    STriangle(SVector( c_boxSize, -c_boxSize,  c_boxSize), SVector( c_boxSize, -c_boxSize, -c_boxSize), SVector( c_boxSize, c_boxSize, -c_boxSize), 11),
    STriangle(SVector( c_boxSize, -c_boxSize,  c_boxSize), SVector( c_boxSize,  c_boxSize,  c_boxSize), SVector( c_boxSize, c_boxSize, -c_boxSize), 11),

    // box wall - bottom
    STriangle(SVector(-c_boxSize, -c_boxSize,  c_boxSize), SVector(-c_boxSize, -c_boxSize, -c_boxSize), SVector( c_boxSize, -c_boxSize, -c_boxSize), 11),
    STriangle(SVector(-c_boxSize, -c_boxSize,  c_boxSize), SVector( c_boxSize, -c_boxSize,  c_boxSize), SVector( c_boxSize, -c_boxSize, -c_boxSize), 11),

    // box wall - top
    STriangle(SVector(-c_boxSize,  c_boxSize,  c_boxSize), SVector(-c_boxSize,  c_boxSize, -c_boxSize), SVector( c_boxSize,  c_boxSize, -c_boxSize), 11),
    STriangle(SVector(-c_boxSize,  c_boxSize,  c_boxSize), SVector( c_boxSize,  c_boxSize,  c_boxSize), SVector( c_boxSize,  c_boxSize, -c_boxSize), 11),

    // box wall - behind
    STriangle(SVector(-c_boxSize, -c_boxSize, -c_boxSize), SVector( c_boxSize, -c_boxSize, -c_boxSize), SVector( c_boxSize,  c_boxSize, -c_boxSize), 11),
    STriangle(SVector(-c_boxSize, -c_boxSize, -c_boxSize), SVector(-c_boxSize,  c_boxSize, -c_boxSize), SVector( c_boxSize,  c_boxSize, -c_boxSize), 11)
);

// Lights
auto c_pointLights = make_array(
    SPointLight(SVector(0.5f, 0.1f, 0.0f), SVector(50.0f, 10.0f, 10.0f)),   // red
    SPointLight(SVector(0.3f, -0.3f, 0.0f), SVector(10.0f, 50.0f, 10.0f)), // green
    SPointLight(SVector(-0.3f, 0.1f, -1.0f), SVector(10.0f, 10.0f, 50.0f))  // blue
);

//=================================================================================
static SVector CameraRight ()
{
    SVector cameraFwd = c_cameraAt - c_cameraPos;
    Normalize(cameraFwd);

    SVector cameraRight = Cross(SVector(0.0f, 1.0f, 0.0f), cameraFwd);
    Normalize(cameraRight);

    SVector cameraUp = Cross(cameraFwd, cameraRight);
    Normalize(cameraUp);

    return cameraRight;
}

//=================================================================================
static SVector CameraUp ()
{
    SVector cameraFwd = c_cameraAt - c_cameraPos;
    Normalize(cameraFwd);

    SVector cameraRight = Cross(SVector(0.0f, 1.0f, 0.0f), cameraFwd);
    Normalize(cameraRight);

    SVector cameraUp = Cross(cameraFwd, cameraRight);
    Normalize(cameraUp);

    return cameraUp;
}

//=================================================================================
static SVector CameraFwd ()
{
    SVector cameraFwd = c_cameraAt - c_cameraPos;
    Normalize(cameraFwd);

    SVector cameraRight = Cross(SVector(0.0f, 1.0f, 0.0f), cameraFwd);
    Normalize(cameraRight);

    SVector cameraUp = Cross(cameraFwd, cameraRight);
    Normalize(cameraUp);

    return cameraFwd;
}

//=================================================================================
static const size_t c_numPixels = c_imageWidth * c_imageHeight;
static const float c_aspectRatio = float(c_imageWidth) / float(c_imageHeight);
static const float c_cameraHorizFOV = c_cameraVerticalFOV * c_aspectRatio;
static const float c_windowTop = tan(c_cameraVerticalFOV / 2.0f) * c_nearDist;
static const float c_windowRight = tan(c_cameraHorizFOV / 2.0f) * c_nearDist;
static const SVector c_cameraRight = CameraRight();
static const SVector c_cameraUp = CameraUp();
static const SVector c_cameraFwd = CameraFwd();

//=================================================================================
bool AnyIntersection (const SVector& a, const SVector& dir, float length, TObjectID ignoreObjectID = c_invalidObjectID)
{
    SCollisionInfo collisionInfo;
    collisionInfo.m_maxCollisionTime = length;
    for (const SSphere& s : c_spheres)
    {
        if (RayIntersects(a, dir, s, collisionInfo, ignoreObjectID) && c_materials[(size_t)collisionInfo.m_materialID].m_blocksLight)
            return true;
    }
    for (const STriangle& t : c_triangles)
    {
        if (RayIntersects(a, dir, t, collisionInfo, ignoreObjectID) && c_materials[(size_t)collisionInfo.m_materialID].m_blocksLight)
            return true;
    }

    return false;
}

//=================================================================================
bool ClosestIntersection (const SVector& rayPos, const SVector& rayDir, SCollisionInfo& collisionInfo, TObjectID ignoreObjectID = c_invalidObjectID)
{
    bool ret = false;
    for (const SSphere& s : c_spheres)
        ret |= RayIntersects(rayPos, rayDir, s, collisionInfo, ignoreObjectID);
    for (const STriangle& t : c_triangles)
        ret |= RayIntersects(rayPos, rayDir, t, collisionInfo, ignoreObjectID);
    return ret;
}

//=================================================================================
SVector L_out (const SCollisionInfo& X, const SVector& outDir, size_t bouncesLeft)
{
    // if no bounces left, return black / darkness
    if (bouncesLeft == 0)
        return SVector();

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
        if (!AnyIntersection(X.m_intersectionPoint, dirToLight, distToLight, X.m_objectID))
        {
            SVector biradiance = Biradiance(pointLight, X.m_intersectionPoint);
            // TODO: scattering stuff, instead of just using diffuse and the dot product thing / test (he abs' the dot product...)
            float dp = Dot(dirToLight, X.m_surfaceNormal);
            if (dp > 0.0f)
                ret += biradiance * material.m_diffuse * abs(dp);
        }
    }

    // add reflection.
    // Temp til I get BRDFs / BSDFs worked out.
    if (NotZero(material.m_reflection))
    {
        SCollisionInfo collisionInfo;
        SVector reflectVector = Reflect(-outDir, X.m_surfaceNormal);
        if (ClosestIntersection(X.m_intersectionPoint, reflectVector, collisionInfo, X.m_objectID))
            ret += material.m_reflection * L_out(collisionInfo, -reflectVector, bouncesLeft - 1);
    }

    return ret;
}

//=================================================================================
SVector L_in (const SVector& X, const SVector& dir)
{
    // if this ray doesn't hit anything, return black / darkness
    SCollisionInfo collisionInfo;
    if (!ClosestIntersection(X, dir, collisionInfo))
        return SVector();

    // else, return the amount of light coming towards us from the object we hit
    return L_out(collisionInfo, -dir, c_maxBounces);
}

//=================================================================================
void RenderPixel(float u, float v, SVector& pixel)
{
    // make (u,v) go from [-1,1] instead of [0,1]
    u = u * 2.0f - 1.0f;
    v = v * 2.0f - 1.0f;

    // find where the ray hits the near plane, and normalize that vector to get the ray direction.
    SVector rayStart = c_cameraPos + c_cameraFwd * c_nearDist;
    rayStart += c_cameraRight * c_windowRight * u;
    rayStart += c_cameraUp * c_windowTop * v;
    SVector rayDir = rayStart - c_cameraPos;
    Normalize(rayDir);

    // our pixel is the amount of light coming in to the position on our near plane from the ray direction.
    pixel = L_in(rayStart, rayDir);
}

//=================================================================================
void ThreadFunc (SImageDataRGBF32& image)
{
    static std::atomic<size_t> g_currentPixelIndex(-1);

    // render individual pixels across multiple threads until we run out of pixels to do
    size_t pixelIndex = ++g_currentPixelIndex;
    while (pixelIndex < image.m_pixels.size())
    {
        // get the current pixel's UV and actual memory location
        size_t x = pixelIndex % image.m_width;
        size_t y = pixelIndex / image.m_height;
        float xPercent = (float)x / (float)image.m_width;
        float yPercent = (float)y / (float)image.m_height;
        RGB_F32& pixel = image.m_pixels[pixelIndex];

        // render the current pixel by averaging (possibly) multiple (possibly) jittered samples.
        // jitter by +/- half a pixel.
        for (size_t i = 0; i < c_samplesPerPixel; ++i)
        {
            float jitterX = 0.0f;
            float jitterY = 0.0f;
            if (c_jitterSamples)
            {
                jitterX = (RandomFloat() - 0.5f) / (float)image.m_width;
                jitterY = (RandomFloat() - 0.5f) / (float)image.m_height;
            }

            SVector out;
            RenderPixel(xPercent + jitterX, yPercent + jitterY, out);

            pixel[0] += out.m_x / (float)c_samplesPerPixel;
            pixel[1] += out.m_y / (float)c_samplesPerPixel;
            pixel[2] += out.m_z / (float)c_samplesPerPixel;
        }

        // apply SRGB correction
        pixel[0] = powf(pixel[0], 1.0f / 2.2f);
        pixel[1] = powf(pixel[1], 1.0f / 2.2f);
        pixel[2] = powf(pixel[2], 1.0f / 2.2f);

        // move to next pixel
        pixelIndex = ++g_currentPixelIndex;
    }
}
 
//=================================================================================
int main (int argc, char **argv)
{
    // make an RGB f32 texture.
    // lower left of image is (0,0).
    SImageDataRGBF32 image_RGB_F32(c_imageWidth, c_imageHeight);

    // Render to the image
    {
        STimer Timer;

        // spin up some threads to do work, and wait for them to be finished.
        size_t numThreads = c_forceSingleThreaded ? 1 : std::thread::hardware_concurrency();
        printf("Spinning up %i threads to make a %i x %i image\r\n", numThreads, c_imageWidth, c_imageHeight);
        std::vector<std::thread> threads;
        threads.resize(numThreads);
        for (std::thread& t : threads)
            t = std::thread(ThreadFunc, std::ref(image_RGB_F32));
        for (std::thread& t : threads)
            t.join();
        printf("Render Time = ");
    }

    // Convert from RGB floating point to BGR u8
    SImageDataBGRU8 image_BGR_U8(c_imageWidth, c_imageHeight);
    for (size_t y = 0; y < c_imageHeight; ++y)
    {
        RGB_F32 *src = &image_RGB_F32.m_pixels[y*image_RGB_F32.m_width];
        BGR_U8 *dest = (BGR_U8*)&image_BGR_U8.m_pixels[y*image_BGR_U8.m_pitch];
        for (size_t x = 0; x < c_imageWidth; ++x)
        {
            (*dest)[0] = uint8(Clamp((*src)[2] * 255.0f, 0.0f, 255.0f));
            (*dest)[1] = uint8(Clamp((*src)[1] * 255.0f, 0.0f, 255.0f));
            (*dest)[2] = uint8(Clamp((*src)[0] * 255.0f, 0.0f, 255.0f));
            ++src;
            ++dest;
        }
    }

    // save the image
    SaveImage("out.bmp", image_BGR_U8);
    return 0;
}

/*

NEXT:

* make it recursive with a maximum bounce depth. bounce randomly in positive hemisphere.  May need scattering function, including impulse support!

* then get rid of point lights and just use emissive objects instead?
 * how does light falloff over distance work in a path tracer?
 * i think it'll just work because fewer samples will hit the light source

* at home there is a weird vertical line when doing 1 sample per pixel and no jitter.
 * see if it happens at work too
 * there's a horizontal one too when the camera is only looking down z axis
 * goes away when we stop testing against triangles.  Only in ClosestIntersection, the other one can check!

GRAPHICS FEATURES:
* better source of random numbers than rand
* scattering function
* importance sampling
* dont forget to tone map to get from whatever floating point values back to 0..1 before going to u8
 * saturate towards white!
* bloom (post process)
* other primitive types
* CSG
* refraction
* beer's law / internal reflection stuff
* participating media (fog)
* blue noise sampling?
 * try different jittering patterns
 * could even try blue noise dithered thing!
* subsurface scattering
* bokeh
* depth of field
* motion blur
* load and render meshes
* textures
* bump mapping
? look up "volumetric path tracing"?  https://en.wikipedia.org/wiki/Volumetric_path_tracing
* area lights and image based lighting
* chromatic abberation etc (may need to do frequency sampling!!)
* adaptive rendering? render at low res, then progressively higher res? look into how that works.
* red / blue 3d glasses mode
? how to address color banding? or is there even banding?
? linearly transformed cosines?
* ggx and spherical harmonics
* ccvt sampling and other stuff from "rolling the dice" siggraph talk

SCENE:
* add a skybox?

OTHER:
* do TODO's in code files
* visualize # of raybounces, instead of colors, for complexity analysis?
 * maybe defines or settings to do this?
 * also visualize normals and reflection bounces or something?
* make it so you can give some kind of identifier to materials, that generates an enum for use in object definitions.  Likely need to make materials into a macro list thing then!
 * then, can make the material id passed to objects be an enum class for strong enforcement!
 * could also maybe have a #define to toggle that makes a sphere for each point light?
* maybe a get material by ID function?
* make it display progress in a window as it goes?
 * might need to double buffer or something
 * make it so when you click on a pixel, you can step through it to see what it's doing
* make it so you can animate things & camera over time at a specified frame rate.  write each frame to disk. combine with ffmpeg to make videos!
* maybe a blog post?
* make width / height be command line options?
* aspect ratio support is weird. it stretches images in a funny way.  may be correct?


*/