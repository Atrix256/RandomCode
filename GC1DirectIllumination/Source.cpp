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
#include "Utils.h"
#include "STimer.h"
#include "Random.h"

//=================================================================================
//                                 SETTINGS
//=================================================================================

// image size
static const size_t c_imageWidth = 512;
static const size_t c_imageHeight = 512;

// sampling
static const size_t c_samplesPerPixel = 100;
static const bool c_jitterSamples = true && (c_samplesPerPixel > 1);
static const size_t c_maxBounces = 4;
static const float c_brightness = 1.0f;

// threading toggle
static const bool c_forceSingleThreaded = false;

// camera - assumes no roll, and that (0,1,0) is up
static const SVector c_cameraPos = { 0.1f, 0.1f, -4.0f };
static const SVector c_cameraAt = { 0.0f, 0.0f, 0.0f };
static const float c_nearDist = 0.1f;
static const float c_cameraVerticalFOV = 60.0f * c_pi / 180.0f;

// Materials
auto c_materials = make_array(
    SMaterial(SVector(0.9f, 0.1f, 0.1f), SVector(), SVector()),                     // matte red
    SMaterial(SVector(0.1f, 0.9f, 0.1f), SVector(), SVector()),                     // matte green
    SMaterial(SVector(0.1f, 0.1f, 0.9f), SVector(), SVector()),                     // matte blue
    SMaterial(SVector(0.1f, 0.9f, 0.9f), SVector(), SVector()),                     // matte teal
    SMaterial(SVector(0.9f, 0.1f, 0.9f), SVector(), SVector()),                     // matte magenta
    SMaterial(SVector(0.9f, 0.9f, 0.1f), SVector(), SVector()),                     // matte yellow
    SMaterial(SVector(0.9f, 0.9f, 0.9f), SVector(), SVector()),                     // matte white
    SMaterial(SVector(0.01f, 0.01f, 0.01f), SVector(), SVector(1.0f, 1.0f, 1.0f)),  // chrome
    SMaterial(SVector(), SVector(0.9f, 0.1f, 0.1f), SVector()),                     // emissive red
    SMaterial(SVector(), SVector(0.1f, 0.9f, 0.1f), SVector()),                     // emissive green
    SMaterial(SVector(), SVector(0.1f, 0.1f, 0.9f), SVector()),                     // emissive blue
    SMaterial(SVector(0.5f, 0.01f, 0.01f), SVector(), SVector(0.1f, 0.1f, 0.1f))    // walls
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
static const float c_brightnessAdjust = c_brightness / (float)c_samplesPerPixel;

//=================================================================================
bool AnyIntersection (const SVector& a, const SVector& dir, float length, TObjectID ignoreObjectID = c_invalidObjectID)
{
    SCollisionInfo collisionInfo;
    collisionInfo.m_maxCollisionTime = length;
    for (const SSphere& s : c_spheres)
    {
        if (RayIntersects(a, dir, s, collisionInfo, ignoreObjectID))
            return true;
    }
    for (const STriangle& t : c_triangles)
    {
        if (RayIntersects(a, dir, t, collisionInfo, ignoreObjectID))
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

    SVector newRayDir = RandomUnitVectorInHemisphere(X.m_surfaceNormal);

    SCollisionInfo collisionInfo;
    if (ClosestIntersection(X.m_intersectionPoint, newRayDir, collisionInfo, X.m_objectID))
    {
        float cos_theta = Dot(newRayDir, X.m_surfaceNormal);

        SVector BRDF = 2.0f * material.m_diffuse * cos_theta; // TODO: name it reflectance, not diffuse!

        ret += BRDF * L_out(collisionInfo, -newRayDir, bouncesLeft - 1);
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
void ReportProgress (size_t index, size_t max)
{
    static int lastProgress = -1;
    int progress = int(100.0f * float(index) / float(max));

    if (progress == lastProgress)
        return;

    lastProgress = progress;
    printf("\r%i%%", progress);
}

//=================================================================================
void ThreadFunc (SImageDataRGBF32& image)
{
    static std::atomic<size_t> g_currentPixelIndex(-1);

    // render individual pixels across multiple threads until we run out of pixels to do
    size_t pixelIndex = ++g_currentPixelIndex;
    bool firstThread = pixelIndex == 0;
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

            pixel[0] += out.m_x * c_brightnessAdjust;
            pixel[1] += out.m_y * c_brightnessAdjust;
            pixel[2] += out.m_z * c_brightnessAdjust;
        }

        // apply SRGB correction
        pixel[0] = powf(pixel[0], 1.0f / 2.2f);
        pixel[1] = powf(pixel[1], 1.0f / 2.2f);
        pixel[2] = powf(pixel[2], 1.0f / 2.2f);

        // move to next pixel
        pixelIndex = ++g_currentPixelIndex;

        // first thread reports progress, show what percent we are at
        if (firstThread)
            ReportProgress(pixelIndex, image.m_pixels.size());
    }

    // first thread reports progress, show 100%
    if (firstThread)
        ReportProgress(1, 1);
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
        printf("\nRender Time = ");
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
    WaitForEnter();
    return 0;
}

/*

NEXT:

GRAPHICS FEATURES:
* smallpt handles glass vs mirrors vs diffuse surfaces differently
 * https://drive.google.com/file/d/0B8g97JkuSSBwUENiWTJXeGtTOHFmSm51UC01YWtCZw/view
* implement roughness somehow
* try mixing direct illumination with monte carlo like this: https://www.shadertoy.com/view/4tl3z4
* better source of random numbers than rand
* scattering function
* importance sampling
* ! multiple importance sampling
* dont forget to tone map to get from whatever floating point values back to 0..1 before going to u8
 * saturate towards white!
* bloom (post process)
* other primitive types?
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
* motion blur (monte carlo sample in time, not just in space)
* load and render meshes
* textures
* bump mapping
? look up "volumetric path tracing"?  https://en.wikipedia.org/wiki/Volumetric_path_tracing
* area lights and image based lighting? this may just work, by having emissive surfaces / textures.
* chromatic abberation etc (may need to do frequency sampling!!)
* adaptive rendering? render at low res, then progressively higher res? look into how that works.
* red / blue 3d glasses mode
? how to address color banding? or is there even banding?
? linearly transformed cosines?
* ggx and spherical harmonics
* ccvt sampling and other stuff from "rolling the dice" siggraph talk
* russian roullette
 * only do if depth > 5
 * roll a random number.  If number > max color component of surface, return emissive color (or black if not allowing emissive)
* could look at smallpt line by line for parity
 * https://drive.google.com/file/d/0B8g97JkuSSBwUENiWTJXeGtTOHFmSm51UC01YWtCZw/view
* spatial acceleration structure could be helpful perhaps, especially when more objects added
* get importance sampling working, to get reflection working for scattering / BRDF
* importance sampling: https://inst.eecs.berkeley.edu/~cs294-13/fa09/lectures/scribe-lecture5.pdf
* make it so you can render multiple frames and it puts them together into a video

SCENE:
* add a skybox?

OTHER:
* at home there is a weird vertical line when doing 1 sample per pixel and no jitter.
 * see if it happens at work too
 * there's a horizontal one too when the camera is only looking down z axis
 * goes away when we stop testing against triangles.  Only in ClosestIntersection, the other one can check!
* maybe make collision tests not tell you if the collision was from inside or not. dot product of ray vs normal can tell you that, for things that care to know.
* make filename be based on resolution, samples and bounce count?
* make it print out resolution, samples, bounce count, primitive count in the window as it's processing
* make it print an estimated time remaining of render based on percentage done and how long it took to get there?
* try to make it so you give a thread an entire row to do.  May be faster?
* show a percentage done.  Could increment a uint64 each time you do a sample. use that as a percentage.
* Implement RandomUnitVectorInHemisphere better
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
* profile with sleepy to see where the time is going!

*/