#include <array>
#include <atomic>
#include <vector>
#include <thread>
#include <random>
#include <windows.h> // for bitmap headers

typedef std::array<float, 3> TVector3;
typedef uint8_t uint8;
typedef std::array<uint8, 3> TPixelBGRU8;
typedef std::array<float, 3> TPixelRGBF32;
const float c_pi = 3.14159265359f;

struct SMaterial
{
    SMaterial (const TPixelRGBF32& emissive, const TPixelRGBF32& diffuse )
        : m_emissive(emissive)
        , m_diffuse(diffuse)
    { }
    TPixelRGBF32    m_emissive;
    TPixelRGBF32    m_diffuse;
};

struct SSphere
{
    TVector3        m_position;
    float           m_radius;
    SMaterial       m_material;
};

struct STriangle
{
    STriangle (const TVector3& a, const TVector3& b, const TVector3& c, const SMaterial& material);

    TVector3    m_a, m_b, m_c;
    SMaterial   m_material;

    // calculated!
    TVector3    m_normal;
};

struct SRayHitInfo
{
    SRayHitInfo ()
        : m_material(nullptr)
        , m_collisionTime(-1.0f)
    { }

    const SMaterial*    m_material;
    TVector3            m_intersectionPoint;
    TVector3            m_surfaceNormal;
    float               m_collisionTime;
};

//=================================================================================
// User tweakable parameters
//=================================================================================

// image size
const size_t c_imageWidth = 512;
const size_t c_imageHeight = 512;

// sampling parameters
const size_t c_samplesPerPixel = 1000;
const size_t c_numBounces = 3;
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

const TVector3 c_rayMissColor = {0.0f, 0.0f, 0.0f};

//=================================================================================
//=================================================================================

std::atomic<size_t> g_currentPixelIndex(-1);
std::vector<TPixelRGBF32> g_pixels;

//=================================================================================
// Vector operations
inline float LengthSq (const TVector3& v)
{
    return (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
}

inline float Length (const TVector3& v)
{
    return sqrt(LengthSq(v));
}

inline TVector3 Normalize (const TVector3& v)
{
    float len = Length(v);
    return
    {
        v[0] / len,
        v[1] / len,
        v[2] / len
    };
}

inline TVector3 operator+ (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] + b[0],
        a[1] + b[1],
        a[2] + b[2]
    };
}

inline TVector3 operator- (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] - b[0],
        a[1] - b[1],
        a[2] - b[2]
    };
}

inline void operator+= (TVector3& a, const TVector3& b)
{
    a[0] += b[0];
    a[1] += b[1];
    a[2] += b[2];
}

inline TVector3 operator* (const TVector3& a, const TVector3& b)
{
    return
    {
        a[0] * b[0],
        a[1] * b[1],
        a[2] * b[2]
    };
}

inline TVector3 operator* (float b, const TVector3& a)
{
    return
    {
        a[0] * b,
        a[1] * b,
        a[2] * b
    };
}

inline TVector3 operator* (const TVector3& a, float b)
{
    return
    {
        a[0] * b,
        a[1] * b,
        a[2] * b
    };
}

inline TVector3 operator/ (const TVector3& a, float b)
{
    return
    {
        a[0] / b,
        a[1] / b,
        a[2] / b
    };
}

inline void operator*= (TVector3& a, float b)
{
    a[0] *= b;
    a[1] *= b;
    a[2] *= b;
}

inline TVector3 operator - (const TVector3& a)
{
    return
    {
        -a[0], -a[1], -a[2]
    };
}

inline float Dot (const TVector3& a, const TVector3& b)
{
    return
        a[0] * b[0] +
        a[1] * b[1] +
        a[2] * b[2];
}

inline TVector3 Cross (const TVector3& a, const TVector3& b)
{
    return
    {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}

//=================================================================================
STriangle::STriangle (const TVector3& a, const TVector3& b, const TVector3& c, const SMaterial& material)
    : m_material(material)
    , m_a(a)
    , m_b(b)
    , m_c(c)
{
    TVector3 e1 = m_b - m_a;
    TVector3 e2 = m_c - m_a;
    m_normal = Normalize(Cross(e1, e2));
}

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
inline float Clamp (float v, float min, float max)
{
    if (v < min)
        return min;
    else if (v > max)
        return max;
    else
        return v;
}

// from 0 to 1
float RandomFloat ()
{
    /*
    // Xorshift random number algorithm invented by George Marsaglia
    static uint32_t rng_state = 0xf2eec0de;
    rng_state ^= (rng_state << 13);
    rng_state ^= (rng_state >> 17);
    rng_state ^= (rng_state << 5);
    return float(rng_state) * (1.0f / 4294967296.0f);
    */

    // alternately, using a standard c++ prng
    static std::random_device rd;
    static std::mt19937 mt(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(mt);
}

float RandomFloat (float min, float max)
{
    return min + (max - min) * RandomFloat();
}

//=================================================================================
inline TVector3 UniformSampleHemisphere (const TVector3& N)
{
    // adapted from @lh0xfb on twitter: https://github.com/gheshu/gputracer/blob/master/depth.glsl
    TVector3 dir;
    do
    {
        dir[0] = RandomFloat(-1.0f, 1.0f);
        dir[1] = RandomFloat(-1.0f, 1.0f);
        dir[2] = RandomFloat(-1.0f, 1.0f);
    } while (LengthSq(dir) > 1.0f);

    if (Dot(dir, N) <= 0.0f)
        dir *= -1.0f;

    dir = Normalize(dir);
    return dir;
}

//=================================================================================
bool RayIntersects (const TVector3& rayPos, const TVector3& rayDir, const SSphere& sphere, SRayHitInfo& info)
{
    //get the vector from the center of this circle to where the ray begins.
    TVector3 m = rayPos - sphere.m_position;

    //get the dot product of the above vector and the ray's vector
    float b = Dot(m, rayDir);

    float c = Dot(m, m) - sphere.m_radius * sphere.m_radius;

    //exit if r's origin outside s (c > 0) and r pointing away from s (b > 0)
    if (c > 0.0 && b > 0.0)
        return false;

    //calculate discriminant
    float discr = b * b - c;

    //a negative discriminant corresponds to ray missing sphere
    if (discr <= 0.0)
        return false;

    //ray now found to intersect sphere, compute smallest t value of intersection
    float collisionTime = -b - sqrt(discr);

    //if t is negative, ray started inside sphere so clamp t to zero and remember that we hit from the inside
    if (collisionTime < 0.0)
        collisionTime = -b + sqrt(discr);

    //enforce a max distance if we should
    if (info.m_collisionTime >= 0.0 && collisionTime > info.m_collisionTime)
        return false;

    TVector3 normal = Normalize((rayPos + rayDir * collisionTime) - sphere.m_position);

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    if (Dot(normal, rayDir) > 0.0f)
        normal *= -1.0f;

    info.m_collisionTime = collisionTime;
    info.m_intersectionPoint = rayPos + rayDir * collisionTime;
    info.m_material = &sphere.m_material;
    info.m_surfaceNormal = normal;
    return true;
}

//=================================================================================
bool RayIntersects (const TVector3& rayPos, const TVector3& rayDir, const STriangle& triangle, SRayHitInfo& info)
{
    // This function adapted from GraphicsCodex.com

    /* If ray P + tw hits triangle V[0], V[1], V[2], then the function returns true,
    stores the barycentric coordinates in b[], and stores the distance to the intersection
    in t. Otherwise returns false and the other output parameters are undefined.*/

    // Edge vectors
    TVector3 e_1 = triangle.m_b - triangle.m_a;
    TVector3 e_2 = triangle.m_c - triangle.m_a;

    const TVector3& q = Cross(rayDir, e_2);
    const float a = Dot(e_1, q);

    if (abs(a) == 0.0f)
        return false;

    const TVector3& s = (rayPos - triangle.m_a) / a;
    const TVector3& r = Cross(s, e_1);
    TVector3 b; // b is barycentric coordinates
    b[0] = Dot(s,q);
    b[1] = Dot(r, rayDir);
    b[2] = 1.0f - b[0] - b[1];
    // Intersected outside triangle?
    if ((b[0] < 0.0f) || (b[1] < 0.0f) || (b[2] < 0.0f)) return false;
    float t = Dot(e_2,r);
    if (t < 0.0f)
        return false;

    //enforce a max distance if we should
    if (info.m_collisionTime >= 0.0 && t > info.m_collisionTime)
        return false;

    // make sure normal is facing opposite of ray direction.
    // this is for if we are hitting the object from the inside / back side.
    TVector3 normal = triangle.m_normal;
    if (Dot(triangle.m_normal, rayDir) > 0.0f)
        normal *= -1.0f;

    info.m_collisionTime = t;
    info.m_intersectionPoint = rayPos + rayDir * t;
    info.m_material = &triangle.m_material;
    info.m_surfaceNormal = normal;
    return true;
}

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
        return c_rayMissColor;

    // start with emissive lighting
    TVector3 ret = X.m_material->m_emissive;

    // add in random recursive samples for global illumination
    {
        const float pdf = 1.0f / (2 * c_pi);

        TVector3 newRayDir = UniformSampleHemisphere(X.m_surfaceNormal);
        SRayHitInfo info;
        if (ClosestIntersection(X.m_intersectionPoint + newRayDir * c_rayBounceEpsilon, newRayDir, info))
        {
            ret += Dot(newRayDir, X.m_surfaceNormal) * 2.0f * L_out(info, -newRayDir, bouncesLeft - 1) * X.m_material->m_diffuse / pdf;
        }
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
void ThreadFunc ()
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

        // render the pixel by taking multiple samples and averaging them
        for (size_t i = 0; i < c_samplesPerPixel; ++i)
        {
            TPixelRGBF32 sample;
            RenderPixel(u, v, sample);
            pixel += (sample - pixel) / float(i + 1.0f);
        }

        // move to next pixel
        pixelIndex = ++g_currentPixelIndex;

        // report our percent done progress (from a single thread only)
        if (firstThread)
        {
            int newPercent = int(1000.0f * float(pixelIndex) / float(c_numPixels));
            if (lastPercent != newPercent)
            {
                printf("\r%0.1f%%", float(newPercent) / 10.0f);
                lastPercent = newPercent;
            }
        }
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

    // spin up some threads to do the rendering work
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    threads.resize(numThreads);
    for (std::thread& t : threads)
        t = std::thread(ThreadFunc);

    // wait for the threads to be done
    for (std::thread& t : threads)
        t.join();
    auto end = std::chrono::high_resolution_clock::now();

    // save the image as out.bmp
    if (!SaveImage())
        printf("Could not save image.\n");

    // report how long it took
    std::chrono::duration<double> diff = end - start;
    printf("\nRendering took %0.1f seconds.\n", diff.count());
    return 0;
}

/*

TODO:
* show time elapsed and time estimated remaining

* do the furnace test to make sure it comes out ok

* use trig for UniformSampleHemisphere instead of looping
* make a #define for direct lighting only (to show for comparison)

* the resulting image is pretty noisy for 5000 spp, why is that?
 * try cosine weighted to see if it helps?
 * also could try jittering
 * could also try it in other renderer to see how it turns out

* note on blog how windows likes to cache images if you are viewing with the windows image viewer! delete file or zoom in / out.


*/