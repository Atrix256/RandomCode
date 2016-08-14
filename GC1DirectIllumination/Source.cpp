#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <thread>
#include <atomic>
#include <windows.h>  // for bitmap headers.  Sorry non windows people!

//=================================================================================

const float c_pi = 3.14159265359f;

//=================================================================================
struct SVector
{
    SVector (float x = 0.0f, float y = 0.0f, float z = 0.0f)
        : m_x(x)
        , m_y(y)
        , m_z(z)
    {}

    float m_x;
    float m_y;
    float m_z;
};

SVector operator * (const SVector& a, float f)
{
    return SVector(a.m_x*f, a.m_y*f, a.m_z*f);
}

SVector operator + (const SVector& a, const SVector& b)
{
    return SVector(a.m_x + b.m_x, a.m_y + b.m_y, a.m_z + b.m_z);
}

SVector operator - (const SVector& a, const SVector& b)
{
    return SVector(a.m_x - b.m_x, a.m_y - b.m_y, a.m_z - b.m_z);
}

SVector operator += (SVector& a, const SVector& b)
{
    a.m_x += b.m_x;
    a.m_y += b.m_y;
    a.m_z += b.m_z;
    return a;
}

void Normalize (SVector& a)
{
    float len = sqrt((a.m_x * a.m_x) + (a.m_y * a.m_y) + (a.m_z * a.m_z));
    a.m_x /= len;
    a.m_y /= len;
    a.m_z /= len;
}

float Dot (const SVector& a, const SVector& b)
{
    return
        a.m_x*b.m_x +
        a.m_y*b.m_y +
        a.m_z*b.m_z;
}

//=================================================================================
static const size_t c_invalidMaterialID = -1;
struct SMaterial
{
    SMaterial(SVector diffuse = SVector())
        : m_diffuse(diffuse)
    {
    }

    SVector m_diffuse;
};

//=================================================================================
static const size_t c_invalidObjectID = 0;
size_t g_lastObjectID = c_invalidObjectID;

struct SCollisionInfo
{
    SCollisionInfo ()
        : m_objectID(c_invalidObjectID)
        , m_materialID(c_invalidMaterialID)
        , m_collisionTime(-1.0f)
        , m_fromInside(false)
    {
    }

    size_t  m_objectID;
    size_t  m_materialID;
    float   m_collisionTime;
    bool    m_fromInside;
    SVector m_intersectionPoint;
    SVector m_surfaceNormal;
};

//=================================================================================
struct SSphere
{
    SSphere(SVector position = SVector(), float radius = 0.0f, size_t materialID = 0)
        : m_position(position)
        , m_radius(radius)
        , m_objectID(++g_lastObjectID)
        , m_materialID(materialID)
    {
    }

    SVector m_position;
    float   m_radius;
    size_t  m_objectID;
    size_t  m_materialID;
};

bool RayIntersects (const SVector& rayPos, const SVector& rayDir, const SSphere& sphere)
{
    //get the vector from the center of this circle to where the ray begins.
    SVector m = rayPos - sphere.m_position;

    //get the dot product of the above vector and the ray's vector
    float b = Dot(m, rayDir);

    float c = Dot(m, m) - sphere.m_radius * sphere.m_radius;

    //exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
        return false;

    //calculate discriminant
    float discr = b * b - c;

    //a negative discriminant corresponds to ray missing sphere
    if (discr < 0.0)
        return false;

    return true;
}

bool RayIntersects (const SVector& rayPos, const SVector& rayDir, const SSphere& sphere, SCollisionInfo& info, size_t ignoreObjectId = c_invalidObjectID)
{
    if (ignoreObjectId == sphere.m_objectID)
        return false;

    //get the vector from the center of this circle to where the ray begins.
    SVector m = rayPos - sphere.m_position;

    //get the dot product of the above vector and the ray's vector
    float b = Dot(m, rayDir);

    float c = Dot(m, m) - sphere.m_radius * sphere.m_radius;

    //exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
        return false;

    //calculate discriminant
    float discr = b * b - c;

    //a negative discriminant corresponds to ray missing sphere
    if (discr < 0.0)
        return false;

    //not inside til proven otherwise
    bool fromInside = false;

    //ray now found to intersect sphere, compute smallest t value of intersection
    float collisionTime = -b - sqrt(discr);

    //if t is negative, ray started inside sphere so clamp t to zero and remember that we hit from the inside
    if (collisionTime < 0.0)
    {
        collisionTime = -b + sqrt(discr);
        fromInside = true;
    }

    //enforce a max distance if we should
    if (info.m_collisionTime >= 0.0 && collisionTime > info.m_collisionTime)
        return false;

    // set all the info params since we are garaunteed a hit at this point
    info.m_fromInside = fromInside;
    info.m_collisionTime = collisionTime;
    info.m_materialID = sphere.m_materialID;

    //compute the point of intersection
    info.m_intersectionPoint = rayPos + rayDir * info.m_collisionTime;

    // calculate the normal
    info.m_surfaceNormal = info.m_intersectionPoint - sphere.m_position;
    Normalize(info.m_surfaceNormal);

    // we found a hit!
    info.m_objectID = sphere.m_objectID;
    return true;
}

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
static const float c_nearDist = 0.01f; // TODO: put to 0.1 or something
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
template <size_t WIDTH, size_t HEIGHT, typename PIXELTYPE>
struct SImageData
{
    SImageData ()
    {
        m_pixels = new PIXELTYPE[NumPixels()];
    }

    ~SImageData ()
    {
        delete[] m_pixels;
    }

    static size_t Width () { return WIDTH; }
    static size_t Height () { return HEIGHT; }
    static size_t NumPixels () { return WIDTH*HEIGHT; }
 
    PIXELTYPE* m_pixels;
};

//=================================================================================
// GLOBALS
//=================================================================================

// lower left of image is (0,0)
SImageData<c_imageWidth, c_imageHeight, RGB_F32> g_image_RGB_F32;

static std::atomic<size_t> g_currentPixelIndex(-1);

//=================================================================================
template <size_t WIDTH, size_t HEIGHT>
bool SaveImage (const char *fileName, const SImageData<WIDTH, HEIGHT, BGR_U8> &image)
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
    infoHeader.biWidth = image.Width();
    infoHeader.biHeight = image.Height();
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = image.NumPixels() * sizeof(BGR_U8);
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