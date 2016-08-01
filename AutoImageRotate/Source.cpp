#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <complex>
#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!

const float c_pi = 3.14159265359f;

typedef uint8_t uint8;

#define CLAMP(v, min, max) if (v < min) { v = min; } else if (v > max) { v = max; } 

//-----------------------------------------------------------------------------------------------------------
void RotatePoint (int x, int y, int aroundX, int aroundY, float& outX, float& outY, float rot)
{
    //x' = x cos f - y sin f
    //y' = y cos f + x sin f
    x -= aroundX;
    y -= aroundY;

    outX = float(x) * cos(rot) - float(y) * sin(rot);
    outY = float(y) * cos(rot) + float(x) * sin(rot);

    outX += (float)aroundX;
    outY += (float)aroundY;
}

//-----------------------------------------------------------------------------------------------------------
struct SProgress
{
    SProgress (const char* message, int total) : m_message(message), m_total(total)
    {
        m_amount = 0;
        m_lastPercent = 0;
        printf("%s   0%%", message);

        QueryPerformanceFrequency(&m_freq);
        QueryPerformanceCounter(&m_start);
    }

    ~SProgress ()
    {
        // make it show 100%
        m_amount = m_total;
        Update(0);

        // show how long it took
        LARGE_INTEGER end;
        QueryPerformanceCounter(&end);
        float seconds = ((float)(end.QuadPart - m_start.QuadPart)) / m_freq.QuadPart;
        printf(" (%0.2f seconds)\n", seconds);
    }

    void Update (int delta = 1)
    {
        m_amount += delta;
        int percent = int(100.0f * float(m_amount) / float(m_total));
        if (percent <= m_lastPercent)
            return;

        m_lastPercent = percent;
        printf("%c%c%c%c", 8, 8, 8, 8);
        if (percent < 100)
            printf(" ");
        if (percent < 10)
            printf(" ");
        printf("%i%%", percent);
    }

    int m_lastPercent;
    int m_amount;
    int m_total;
    const char* m_message;

    LARGE_INTEGER m_start;
    LARGE_INTEGER m_freq;
};
 
//-----------------------------------------------------------------------------------------------------------
struct SImageData
{
    SImageData ()
        : m_width(0)
        , m_height(0)
    { }
 
    long m_width;
    long m_height;
    long m_pitch;
    std::vector<uint8> m_pixels;
};

//-----------------------------------------------------------------------------------------------------------
struct SImageDataComplex
{
    SImageDataComplex ()
        : m_width(0)
        , m_height(0)
    { }

    long m_width;
    long m_height;
    std::vector<std::complex<float>> m_pixels;
};

//-----------------------------------------------------------------------------------------------------------
struct SImageDataFloat
{
    SImageDataFloat()
        : m_width(0)
        , m_height(0)
    { }

    long m_width;
    long m_height;
    std::vector<float> m_pixels;
};
 
//-----------------------------------------------------------------------------------------------------------
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
 
    imageData.m_pitch = imageData.m_width*3;
    if (imageData.m_pitch & 3)
    {
        imageData.m_pitch &= ~3;
        imageData.m_pitch += 4;
    }
 
    fclose(file);
    return true;
}
 
//-----------------------------------------------------------------------------------------------------------
bool SaveImage (const char *fileName, const SImageData &image)
{
    // open the file if we can
    FILE *file;
    file = fopen(fileName, "wb");
    if (!file) {
        printf("Could not save %s\n", fileName);
        return false;
    }
 
    // make the header info
    BITMAPFILEHEADER header;
    BITMAPINFOHEADER infoHeader;
 
    header.bfType = 0x4D42;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits = 54;
 
    infoHeader.biSize = 40;
    infoHeader.biWidth = image.m_width;
    infoHeader.biHeight = image.m_height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = image.m_pixels.size();
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

    printf("%s saved\n", fileName);
    return true;
}
 
//-----------------------------------------------------------------------------------------------------------
void ImageToGrey (const SImageData &srcImage, SImageData &destImage)
{
    destImage = srcImage;

    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            const uint8 *src = &srcImage.m_pixels[(y * srcImage.m_pitch) + x * 3];
            uint8 *dest = &destImage.m_pixels[(y * destImage.m_pitch) + x * 3];

            uint8 grey = uint8((float(src[0]) * 0.3f + float(src[1]) * 0.59f + float(src[2]) * 0.11f));
            dest[0] = grey;
            dest[1] = grey;
            dest[2] = grey;
        }
    }
}

//-----------------------------------------------------------------------------------------------------------
std::complex<float> DFTPixel (const SImageData &srcImage, int K, int L)
{
    std::complex<float> ret(0.0f, 0.0f);

    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // Get the pixel value (assuming greyscale) and convert it to [0,1] space
            const uint8 *src = &srcImage.m_pixels[(y * srcImage.m_pitch) + x * 3];
            float grey = float(src[0]) / 255.0f;

            // Add to the sum of the return value
            float v = float(K * x) / float(srcImage.m_width);
            v += float(L * y) / float(srcImage.m_height);
            ret += std::complex<float>(grey, 0.0f) * std::polar<float>(1.0f, -2.0f * c_pi * v);
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------------------------------------
std::complex<float> DFTPixelHorizontalPass (const SImageData &srcImage, int K, int L)
{
    std::complex<float> ret(0.0f, 0.0f);

    for (int x = 0; x < srcImage.m_width; ++x)
    {
        // Get the pixel value (assuming greyscale) and convert it to [0,1] space
        const uint8 *src = &srcImage.m_pixels[(L * srcImage.m_pitch) + x * 3];
        float grey = float(src[0]) / 255.0f;

        // Add to the sum of the return value
        float v = float(K * x) / float(srcImage.m_width);
        ret += std::complex<float>(grey, 0.0f) * std::polar<float>(1.0f, -2.0f * c_pi * v);
    }

    return ret;
}

//-----------------------------------------------------------------------------------------------------------
std::complex<float> DFTPixelVerticalPass (const SImageDataComplex& horizontalPass, int K, int L)
{
    std::complex<float> ret(0.0f, 0.0f);

    for (int y = 0; y < horizontalPass.m_height; ++y)
    {
        // get our data from the horizontal pass.
        // It already has the source image multiplied in, so no need to look up the source data pixel
        const std::complex<float>& hPassData = horizontalPass.m_pixels[y * horizontalPass.m_width + K];

        // Add to the sum of the return value
        float v = float(L * y) / float(horizontalPass.m_height);
        ret += hPassData* std::polar<float>(1.0f, -2.0f * c_pi * v);
    }

    return ret;
}
 
//-----------------------------------------------------------------------------------------------------------
void DFTImage (const SImageData &srcImage, SImageDataComplex &destImage)
{
    // NOTE: this function assumes srcImage is greyscale, so works on only the red component of srcImage.
    // ImageToGrey() will convert an image to greyscale.

    // size the output dft data
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pixels.resize(destImage.m_width*destImage.m_height);

    SProgress progress("DFT:", 2 * srcImage.m_width * srcImage.m_height);

    // Do horizontal passes
    SImageDataComplex horizontalPass;
    horizontalPass = destImage;
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // calculate DFT for that pixel / frequency
            horizontalPass.m_pixels[y * horizontalPass.m_width + x] = DFTPixelHorizontalPass(srcImage, x, y);

            // update progress
            progress.Update();
        }
    }

    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // calculate DFT for that pixel / frequency
            destImage.m_pixels[y * destImage.m_width + x] = DFTPixelVerticalPass(horizontalPass, x, y);

            // update progress
            progress.Update();
        }
    }
}

//-----------------------------------------------------------------------------------------------------------
float GetMagnitudeDataFloat (const SImageDataComplex& srcImage, SImageDataFloat& destImage)
{
    // size the output image
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pixels.resize(destImage.m_width*destImage.m_height);

    // get floating point magnitude data
    float maxMag = 0.0f;
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // Offset the information by half width & height in the positive direction.
            // This makes frequency 0 (DC) be at the image origin, like most diagrams show it.
            int k = (x + srcImage.m_width / 2) % srcImage.m_width;
            int l = (y + srcImage.m_height / 2) % srcImage.m_height;
            const std::complex<float> &src = srcImage.m_pixels[l*srcImage.m_width + k];

            float mag = std::abs(src);
            if (mag > maxMag)
                maxMag = mag;

            destImage.m_pixels[y*destImage.m_width + x] = mag;
        }
    }
    if (maxMag == 0.0f)
        maxMag = 1.0f;
    return maxMag;
}

//-----------------------------------------------------------------------------------------------------------
void GetMagnitudeData (const SImageDataComplex& srcImage, SImageData& destImage)
{
    // size the output image
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pitch = srcImage.m_width * 3;
    if (destImage.m_pitch & 3)
    {
        destImage.m_pitch &= ~3;
        destImage.m_pitch += 4;
    }
    destImage.m_pixels.resize(destImage.m_pitch*destImage.m_height);

    // get the floating point frequency magnitude data
    SImageDataFloat magData;
    float maxMag = GetMagnitudeDataFloat(srcImage, magData);
    if (maxMag == 0.0f)
        maxMag = 1.0f;

    // normalize the magnitude data and send it back in [0, 255], applying a log scale
    const float c = 255.0f / log(1.0f + maxMag);
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            float src = c * log(1.0f + magData.m_pixels[y*magData.m_width + x]);

            uint8 magu8 = uint8(src);

            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = magu8;
            dest[1] = magu8;
            dest[2] = magu8;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MODIFIED BRESENHAM LINE DRAWING
// Adapted from http://blog.demofox.org/2015/01/17/bresenhams-drawing-algorithms/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Generic Line Drawing
// X and Y are flipped for Y maxor axis lines, but the pixel writes are handled correctly due to
// minor and major axis pixel movement
template <typename PIXELTYPE, typename LAMBDA>
void DrawLineMajorAxis(
    PIXELTYPE* pixel,
    int majorAxisPixelMovement,
    int minorAxisPixelMovement,
    int dx,
    int dy,
    const LAMBDA& lambda
)
{
    // calculate some constants
    const int dx2 = dx * 2;
    const int dy2 = dy * 2;
    const int dy2Mindx2 = dy2 - dx2;
 
    // calculate the starting error value
    int Error = dy2 - dx;
 
    // draw the first pixel
    lambda(pixel);
 
    // loop across the major axis
    while (dx--)
    {
        // move on major axis and minor axis
        if (Error > 0)
        {
            pixel += majorAxisPixelMovement + minorAxisPixelMovement;
            Error += dy2Mindx2;
        }
        // move on major axis only
        else
        {
            pixel += majorAxisPixelMovement;
            Error += dy2;
        }
 
        // draw the next pixel
        lambda(pixel);
    }
}
 
// Specialized Line Drawing optimized for horizontal or vertical lines
// X and Y are flipped for Y maxor axis lines, but the pixel writes are handled correctly due to
// minor and major axis pixel movement
template <typename PIXELTYPE, typename LAMBDA>
void DrawLineSingleAxis(PIXELTYPE* pixel, int majorAxisPixelMovement, int dx, const LAMBDA& lambda)
{
    // draw the first pixel
    lambda(pixel);
 
    // loop across the major axis and draw the rest of the pixels
    while (dx--)
    {
        pixel += majorAxisPixelMovement;
        lambda(pixel);
    };
}
 
// Draw an arbitrary line.  Assumes start and end point are within valid range
// pixels is a pointer to where the pixels you want to draw to start aka (0,0)
// pixelStride is the number of items to get from one row of pixels to the next.
// Usually, that is the same as the width of the image you are drawing to, but sometimes is not.
template <typename PIXELTYPE, typename LAMBDA>
void DrawLine(PIXELTYPE* pixels, int pixelStride, int x1, int y1, int x2, int y2, const LAMBDA& lambda)
{
    // calculate our deltas
    int dx = x2 - x1;
    int dy = y2 - y1;
 
    // if the X axis is the major axis
    if (abs(dx) >= abs(dy))
    {
        // if x2 < x1, flip the points to have fewer special cases
        if (dx < 0)
        {
            dx *= -1;
            dy *= -1;
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
 
        // get the address of the pixel at (x1,y1)
        PIXELTYPE* startPixel = &pixels[y1 * pixelStride + x1];
 
        // determine special cases
        if (dy > 0)
            DrawLineMajorAxis(startPixel, 1, pixelStride, dx, dy, lambda);
        else if (dy < 0)
            DrawLineMajorAxis(startPixel, 1, -pixelStride, dx, -dy, lambda);
        else
            DrawLineSingleAxis(startPixel, 1, dx, lambda);
    }
    // else the Y axis is the major axis
    else
    {
        // if y2 < y1, flip the points to have fewer special cases
        if (dy < 0)
        {
            dx *= -1;
            dy *= -1;
            std::swap(x1, x2);
            std::swap(y1, y2);
        }
 
        // get the address of the pixel at (x1,y1)
        PIXELTYPE* startPixel = &pixels[y1 * pixelStride + x1];
 
        // determine special cases
        if (dx > 0)
            DrawLineMajorAxis(startPixel, pixelStride, 1, dy, dx, lambda);
        else if (dx < 0)
            DrawLineMajorAxis(startPixel, pixelStride, -1, dy, -dx, lambda);
        else
            DrawLineSingleAxis(startPixel, pixelStride, dy, lambda);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bicubic texture interpolation
// from http://blog.demofox.org/2015/08/15/resizing-images-with-bicubic-interpolation/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// t is a value that goes from 0 to 1 to interpolate in a C1 continuous way across uniformly sampled data points.
// when t is 0, this will return B.  When t is 1, this will return C.  Inbetween values will return an interpolation
// between B and C.  A and B are used to calculate slopes at the edges.
float CubicHermite(float A, float B, float C, float D, float t)
{
    float a = -A / 2.0f + (3.0f*B) / 2.0f - (3.0f*C) / 2.0f + D / 2.0f;
    float b = A - (5.0f*B) / 2.0f + 2.0f*C - D / 2.0f;
    float c = -A / 2.0f + C / 2.0f;
    float d = B;

    return a*t*t*t + b*t*t + c*t + d;
}

const uint8* GetPixelClamped(const SImageData& image, int x, int y)
{
    CLAMP(x, 0, image.m_width - 1);
    CLAMP(y, 0, image.m_height - 1);
    return &image.m_pixels[(y * image.m_pitch) + x * 3];
}

std::array<uint8, 3> SampleBicubic (const SImageData& image, float u, float v)
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
    // Clamp the values since the curve can put the value below 0 or above 255
    std::array<uint8, 3> ret;
    for (int i = 0; i < 3; ++i)
    {
        float col0 = CubicHermite(p00[i], p10[i], p20[i], p30[i], xfract);
        float col1 = CubicHermite(p01[i], p11[i], p21[i], p31[i], xfract);
        float col2 = CubicHermite(p02[i], p12[i], p22[i], p32[i], xfract);
        float col3 = CubicHermite(p03[i], p13[i], p23[i], p33[i], xfract);
        float value = CubicHermite(col0, col1, col2, col3, yfract);
        CLAMP(value, 0.0f, 255.0f);
        ret[i] = uint8(value);
    }
    return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main (int argc, char **argv)
{
    // get command line params if we can
    bool showUsage = argc < 2;
    char *srcFileName = argv[1];
    if (showUsage)
    {
        printf("Usage: <source>\n\n");
        return 1;
    }

    // trim off file extension from source filename so we can make our other file names
    char baseFileName[1024];
    strcpy(baseFileName, srcFileName);
    for (int i = strlen(baseFileName) - 1; i >= 0; --i)
    {
        if (baseFileName[i] == '.')
        {
            baseFileName[i] = 0;
            break;
        }
    }

    // Load source image if we can
    SImageData srcImage;
    if (!LoadImage(srcFileName, srcImage))
    {
        printf("could not read 24 bit bmp file %s\n\n", srcFileName);
        return 0;
    }
    printf("%s loaded (%i x %i)\n", srcFileName, srcImage.m_width, srcImage.m_height);

    // do DFT on a greyscale version of the image, instead of doing it per color channel
    SImageData greyImage;
    ImageToGrey(srcImage, greyImage);
    SImageDataComplex frequencyData;
    DFTImage(greyImage, frequencyData);

    // save frequency magnitude information
    char outFileName[1024];
    strcpy(outFileName, baseFileName);
    strcat(outFileName, ".dft.bmp");
    SImageData frequencyMagnitudes;
    GetMagnitudeData(frequencyData, frequencyMagnitudes);
    SaveImage(outFileName, frequencyMagnitudes);

    // get the frequency magntiude data in a raw float format
    SImageDataFloat frequencyMagnitudesRaw;
    GetMagnitudeDataFloat(frequencyData, frequencyMagnitudesRaw);

    // do a "radar sweep" of the image, finding which line (angle) has the highest frequency amplitudes.
    const int halfImageWidth = srcImage.m_width / 2;
    const int halfImageHeight = srcImage.m_height / 2;
    float bestAngle = 0.0f;
    float bestValue = -1.0f;
    int srcX, srcY, destX, destY;
    int bestLine[4];

    {
        SProgress progress("Analyzing Image", srcImage.m_width + srcImage.m_height);

        // Test perfectly horizontal, for the case of no rotation needed
        {
            // sum up the amplitude data along the line
            srcX = 0;
            srcY = halfImageHeight;
            destX = srcImage.m_width - 1;
            destY = halfImageHeight;
            float totalMag = 0.0f;
            DrawLine(
                &frequencyMagnitudesRaw.m_pixels[0],
                frequencyMagnitudesRaw.m_width,
                srcX, srcY, destX, destY,
                [&totalMag](const float *pixel)
                {
                    totalMag += *pixel;
                }
            );

            // if this is higher than what we had before, store it off as the new winner
            if (totalMag > bestValue)
            {
                bestLine[0] = srcX;
                bestLine[1] = srcY;
                bestLine[2] = destX;
                bestLine[3] = destY;
                float dx = float(destX - srcX);
                float dy = float(destY - srcY);
                bestAngle = atan2(dy, dx);
                bestValue = totalMag;
            }
        }

        // Test perfectly vertical, for the case of no rotation needed
        {
            // sum up the amplitude data along the line
            srcX = halfImageWidth;
            srcY = 0;
            destX = halfImageWidth;
            destY = srcImage.m_height - 1;
            float totalMag = 0.0f;
            DrawLine(
                &frequencyMagnitudesRaw.m_pixels[0],
                frequencyMagnitudesRaw.m_width,
                srcX, srcY, destX, destY,
                [&totalMag](const float *pixel)
                {
                    totalMag += *pixel;
                }
            );

            // if this is higher than what we had before, store it off as the new winner
            if (totalMag > bestValue)
            {
                bestLine[0] = srcX;
                bestLine[1] = srcY;
                bestLine[2] = destX;
                bestLine[3] = destY;
                float dx = float(destX - srcX);
                float dy = float(destY - srcY);
                bestAngle = atan2(dy, dx);
                bestValue = totalMag;
            }
        }

        // test the horizontal walls
        srcX = 0;
        destX = srcImage.m_width - 1;
        for (int i = 0; i <= srcImage.m_height; ++i)
        {
            // update progress and update our test line
            progress.Update();
            srcY = i;
            destY = srcImage.m_height - 1 - i;

            // sum up the amplitude data along the line
            // make sure to explicitly get the center point (DC)
            float totalMag = 0.0f;
            DrawLine(
                &frequencyMagnitudesRaw.m_pixels[0],
                frequencyMagnitudesRaw.m_width,
                halfImageWidth, halfImageHeight, destX, destY,
                [&totalMag](const float *pixel)
                {
                    totalMag += *pixel;
                }
            );
            DrawLine(
                &frequencyMagnitudesRaw.m_pixels[0],
                frequencyMagnitudesRaw.m_width,
                halfImageWidth, halfImageHeight, srcX, srcY,
                [&totalMag](const float *pixel)
                {
                    totalMag += *pixel;
                }
            );

            // if this is higher than what we had before, store it off as the new winner
            if (totalMag > bestValue)
            {
                bestLine[0] = srcX;
                bestLine[1] = srcY;
                bestLine[2] = destX;
                bestLine[3] = destY;
                float dx = float(destX - srcX);
                float dy = float(destY - srcY);
                bestAngle = atan2(dy, dx);
                bestValue = totalMag;
            }
        }

        // test the vertical walls wall
        srcY = 0;
        destY = srcImage.m_height - 1;
        for (int i = 0; i <= srcImage.m_width; ++i)
        {
            // update progress and update our test line
            progress.Update();
            srcX = i;
            destX = srcImage.m_width - 1 - i;

            // sum up the amplitude data along the line
            // make sure to explicitly get the center point (DC)
            float totalMag = 0.0f;
            DrawLine(
                &frequencyMagnitudesRaw.m_pixels[0],
                frequencyMagnitudesRaw.m_width,
                halfImageWidth, halfImageHeight, destX, destY,
                [&totalMag](const float *pixel)
                {
                    totalMag += *pixel;
                }
            );
            DrawLine(
                &frequencyMagnitudesRaw.m_pixels[0],
                frequencyMagnitudesRaw.m_width,
                halfImageWidth, halfImageHeight, srcX, srcY,
                [&totalMag](const float *pixel)
                {
                    totalMag += *pixel;
                }
            );

            // if this is higher than what we had before, store it off as the new winner
            if (totalMag > bestValue)
            {
                bestLine[0] = srcX;
                bestLine[1] = srcY;
                bestLine[2] = destX;
                bestLine[3] = destY;
                float dx = float(destX - srcX);
                float dy = float(destY - srcY);
                bestAngle = atan2(dy, dx);
                bestValue = totalMag;
            }
        }
    }

    // Draw the winning line and save the image!
    strcpy(outFileName, baseFileName);
    strcat(outFileName, ".dft2.bmp");
    DrawLine(
        (std::array<uint8, 3>*)&frequencyMagnitudes.m_pixels[0],
        frequencyMagnitudes.m_pitch / 3,
        halfImageWidth, halfImageHeight, bestLine[0], bestLine[1],
        [] (std::array<uint8, 3> *pixel)
        {
            (*pixel)[1] = 0;
            (*pixel)[2] = 0;
        }
    );
    DrawLine(
        (std::array<uint8, 3>*)&frequencyMagnitudes.m_pixels[0],
        frequencyMagnitudes.m_pitch / 3,
        halfImageWidth, halfImageHeight, bestLine[2], bestLine[3],
        [] (std::array<uint8, 3> *pixel)
        {
            (*pixel)[0] = 0;
            (*pixel)[1] = 0;
        }
    );
    SaveImage(outFileName, frequencyMagnitudes);

    // calculate and report angle adjustment needed - find nearest 90 degree angle!
    // We don't know if the image is supposed to be horizontal or vertical, so we can only
    // assume that the closest orientation fix is the right one.  AKA we assume the orientation
    // is almost right, but just needs a little help
    float orientationDegrees = bestAngle * 180.0f / c_pi;
    float nearest90degrees = floor((orientationDegrees / 90.0f) + 0.5f) * 90.0f;
    float rotationFixDegrees = nearest90degrees - orientationDegrees;
    printf("The image is oriented at %0.2f degrees and needs a %0.2f degree rotation adjustment.\n", orientationDegrees, rotationFixDegrees);

    // rotate the image.
    // * Loop through an image of the same size
    // * Unrotate the pixel location to find the location on the source image
    // * bicubic interpolate source image pixel to get destination image pixel
    SImageData rotatedImage;
    rotatedImage.m_width = srcImage.m_width;
    rotatedImage.m_height = srcImage.m_height;
    rotatedImage.m_pitch = srcImage.m_width * 3;
    if (rotatedImage.m_pitch & 3)
    {
        rotatedImage.m_pitch &= ~3;
        rotatedImage.m_pitch += 4;
    }
    rotatedImage.m_pixels.resize(rotatedImage.m_pitch*rotatedImage.m_height);
    float unrotation = rotationFixDegrees * c_pi / 180.0f;
    for (int y = 0, yc = rotatedImage.m_height; y < yc; ++y)
    {
        for (int x = 0, xc = rotatedImage.m_width; x < xc; ++x)
        {
            float srcPointX = 0.0f;
            float srcPointY = 0.0f;
            RotatePoint(x, y, rotatedImage.m_width / 2, rotatedImage.m_height / 2, srcPointX, srcPointY, unrotation);
            float u = srcPointX / (float)srcImage.m_width;
            float v = srcPointY / (float)srcImage.m_height;

            // leave out of bounds samples black
            if (u >= 0.0 && v >= 0.0 && u <= 1.0f && v <= 1.0f)
            {
                std::array<uint8, 3> src = SampleBicubic(srcImage, u, v);
                uint8* dest = &rotatedImage.m_pixels[y * rotatedImage.m_pitch + x * 3];

                dest[0] = src[0];
                dest[1] = src[1];
                dest[2] = src[2];
            }
        }
    }

    // save the rotated image
    strcpy(outFileName, baseFileName);
    strcat(outFileName, ".rotated.bmp");
    SaveImage(outFileName, rotatedImage);
    return 0;
}

/*

TODO:
* test2's dft overlay lines seem wrong.  That doesn't look like the best line has won.
* see why it wants to rotate zelda guy.
 * make sure it's doing the perfect 90 degree angles, to be able to catch when it doesn't need rotation
* test2.bmp is rotated the wrong way?
* i think you need to make sure all the lines EXPLICITLY pass through DC.
 * maybe need to do two lines - pos and neg - and add them together.
 * i think this is causing the problems


Blog:
* compare timing of faster DFT / IFT vs before (mention would make it threaded and do FFT)
* not using sliding dot product! don't mention that!
 * mention histogram similarity, and reference search engine post
* get some good images to demonstrate it on (hopefully a little bit larger images?)
* show fft before and after for each image
* make a diagram to show how this works (draw a line through center and mention that's where it samples)

* link to this which talks about how to make it even faster
 * http://dsp.stackexchange.com/questions/32408/faster-way-of-getting-2d-frequency-amplitudes-than-dft

* mention that it goes from N^2 to 2N (or figure out exactly what the diff is?)
 * check here under "how it works": http://homepages.inf.ed.ac.uk/rbf/HIPR2/fourier.htm
 * or just look at code when it's working!

Next: 
* DownScaleImage with sinc interpolation, and notes there!

*/