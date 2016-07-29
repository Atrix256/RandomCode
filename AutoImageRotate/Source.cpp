#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <complex>
#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!

const float c_pi = 3.14159265359f;
const float c_rootTwo = 1.41421356237f;

typedef uint8_t uint8;

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

std::complex<float> DFTPixelHorizontal (const SImageData &srcImage, int K, int L)
{
    // TODO: continue this, checking out the "how it works" section of http://homepages.inf.ed.ac.uk/rbf/HIPR2/fourier.htm
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
 
void DFTImage (const SImageData &srcImage, SImageDataComplex &destImage)
{
    // NOTE: this function assumes srcImage is greyscale, so works on only the red component of srcImage.
    // ImageToGrey() will convert an image to greyscale.

    // size the output dft data
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pixels.resize(destImage.m_width*destImage.m_height);

    SProgress progress("DFT:", srcImage.m_width * srcImage.m_height);

    // Do horizontal passes
    // TODO: i think we need a SImageDataComplex for horizontal pass.
    // TODO: may want to have a vertical pass too, then combine as a 3rd step?
    // TODO: make progress work in this function!
    SImageDataComplex horizontalPass;
    horizontalPass = destImage;
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // calculate DFT for that pixel / frequency
            destImage.m_pixels[y * destImage.m_width + x] = DFTPixelHorizontal(srcImage, x, y);
            // TODO: horizontalPass, not destImage!
        }
    }

    // TODO: continue this! checking out the "how it works" section of http://homepages.inf.ed.ac.uk/rbf/HIPR2/fourier.htm

    /*
 
    // calculate 2d dft (brute force, not using fast fourier transform)
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // calculate DFT for that pixel / frequency
            destImage.m_pixels[y * destImage.m_width + x] = DFTPixel(srcImage, x, y);

            // update progress
            progress.Update();
        }
    }
    */
}

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

    // get floating point magnitude data
    std::vector<float> magArray;
    magArray.resize(srcImage.m_width*srcImage.m_height);
    float maxmag = 0.0f;
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
            if (mag > maxmag)
                maxmag = mag;

            magArray[y*srcImage.m_width + x] = mag;
        }
    }
    if (maxmag == 0.0f)
        maxmag = 1.0f;

    const float c = 255.0f / log(1.0f+maxmag);

    // normalize the magnitude data and send it back in [0, 255]
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            float src = c * log(1.0f + magArray[y*srcImage.m_width + x]);

            uint8 magu8 = uint8(src);

            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = magu8;
            dest[1] = magu8;
            dest[2] = magu8;
        }
    }
}

int main (int argc, char **argv)
{
    float scale = 1.0f;
    int filter = 0;
 
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

    // save magnitude information
    {
        char outFileName[1024];
        strcpy(outFileName, baseFileName);
        strcat(outFileName, ".dtf.bmp");

        SImageData destImage;
        GetMagnitudeData(frequencyData, destImage);
        SaveImage(outFileName, destImage);
    }

    // TODO: rotate image etc

    return 0;
}

/*

TODO:
* Make DFT / IDFT faster!
 * separate axis
 * threaded
 * fast fourier transform from the audio post?

* auto rotate images!
 * fft the image
 * do auto correlation with a rotating line (0-180 degrees)
 * rotate image, using bicubic sampling for interpolation
 * pad rotated image with black




Blog:
* compare timing of faster DFT / IFT vs before
* not using sliding dot product! don't mention that!
 * mention histogram similarity, and reference search engine post
* get some good images to demonstrate it on (hopefully a little bit larger images?)
* show fft before and after for each image

* mention that it goes from N^2 to 2N (or figure out exactly what the diff is?)
 * check here under "how it works": http://homepages.inf.ed.ac.uk/rbf/HIPR2/fourier.htm
 * or just look at code when it's working!

Next: 
* DownScaleImage with sinc interpolation, and notes there!

*/