#define _CRT_SECURE_NO_WARNINGS
 
#include <stdio.h>
#include <stdint.h>
#include <array>
#include <vector>
#include <complex>
#include <windows.h>  // for bitmap headers.  Sorry non windows people!
 
typedef uint8_t uint8;
 
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
            ret += std::complex<float>(grey, 0.0f) * std::polar<float>(1.0f, -2.0f * 3.14159265359f * v);
        }
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

    printf("DFT:   0%%");
 
    // calculate 2d dft (brute force, not using fast fourier transform)
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // offset the sample by half of width / height to make DC (frequency 0) be in the center of the output image
            //int K = (x + srcImage.m_width / 2) % srcImage.m_width;
            //int L = (y + srcImage.m_height / 2) % srcImage.m_height;

            // calculate DFT for that pixel / frequency
            destImage.m_pixels[y * destImage.m_width + x] = DFTPixel(srcImage, x, y);

            // show progress
            int percent = int(100.0f * float(x * srcImage.m_height + y) / float(srcImage.m_width * srcImage.m_height));
            if (percent > 100)
                percent = 100;
            printf("%c%c%c%c", 8, 8, 8, 8);
            if (percent < 100)
                printf(" ");
            if (percent < 10)
                printf(" ");
            printf("%i%%", percent);
        }
    }

    printf("\n");
}

float InverseDFTPixel (const SImageDataComplex &srcImage, int K, int L)
{
    float ret = 0.0f;

    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // Get the pixel value
            const std::complex<float> &src = srcImage.m_pixels[(y * srcImage.m_width) + x];

            // Add to the sum of the return value
            float v = float(K * x) / float(srcImage.m_width);
            v += float(L * y) / float(srcImage.m_height);
            std::complex<float> result = src * std::polar<float>(1.0f, 2.0f * 3.14159265359f * v);

            // TODO: not sure if adding magnitude is correct.  may only want real part? see how round trip works
            // NOTES: using real() i got negative values.  using magnitude, i got values out of range (like, 8)
            float mag = std::abs(result);// sqrt(result.imag()*result.imag() + result.real()*result.real());
            ret += result.real();
            //ret += sqrt(result.imag()*result.imag() + result.real()*result.real());
        }
    }

    // TODO: should this function return uint8? i think so
    return ret / float(srcImage.m_width*srcImage.m_height);
}

void InverseDFTImage (const SImageDataComplex &srcImage, SImageData &destImage)
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

    printf("Inverse DFT:   0%%");

    // calculate inverse 2d dft (brute force, not using fast fourier transform)
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // offset the sample by half of width / height because DC (frequency 0) is in the center of the frequency data
            //int K = (x + srcImage.m_width / 2) % srcImage.m_width;
            //int L = (y + srcImage.m_height / 2) % srcImage.m_height;

            // calculate DFT for that pixel / frequency
            float idft = InverseDFTPixel(srcImage, x, y);

            // make sure the values are in range
            if (idft < 0.0f)
                idft = 0.0f;
            if (idft > 1.0f)
                idft = 1.0;

            uint8 grey = uint8(idft * 255.0f);
            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = grey;
            dest[1] = grey;
            dest[2] = grey;

            // show progress
            int percent = int(100.0f * float(x * srcImage.m_height + y) / float(srcImage.m_width * srcImage.m_height));
            if (percent > 100)
                percent = 100;
            printf("%c%c%c%c", 8, 8, 8, 8);
            if (percent < 100)
                printf(" ");
            if (percent < 10)
                printf(" ");
            printf("%i%%", percent);
        }
    }

    printf("\n");
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
 
    printf("Calculating fourier transform of %s\n\n", srcFileName);

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

    // make our file names
    char magnitudeFileName[1024];
    strcpy(magnitudeFileName, baseFileName);
    strcat(magnitudeFileName, ".dftmag.bmp");

    char greyFileName[1024];
    strcpy(greyFileName, baseFileName);
    strcat(greyFileName, ".grey.bmp");

    char idftFileName[1024];
    strcpy(idftFileName, baseFileName);
    strcat(idftFileName, ".idft.bmp");

    // Load source image if we can
    SImageData srcImage;
    if (LoadImage(srcFileName, srcImage))
    {
        printf("%s loaded (%i x %i)\n", srcFileName, srcImage.m_width, srcImage.m_height);

        // convert to grey and save it out
        SImageData greyImage;
        ImageToGrey(srcImage, greyImage);
        if (SaveImage(greyFileName, greyImage))
            printf("Saved %s\n", greyFileName);
        else
            printf("Could not save %s\n", greyFileName);

        // do DFT and save out magnitude information
        SImageDataComplex frequencyData;
        DFTImage(greyImage, frequencyData);
        SImageData destImage;
        GetMagnitudeData(frequencyData, destImage);
        if (SaveImage(magnitudeFileName, destImage))
            printf("Saved %s\n", magnitudeFileName);
        else
            printf("Could not save %s\n", magnitudeFileName);

        // TODO: modify frequency data and save out the result

        // inverse dft the modified frequency and save the result
        SImageData modifiedImage;
        InverseDFTImage(frequencyData, modifiedImage);
        if (SaveImage(idftFileName, modifiedImage))
            printf("Saved %s\n", idftFileName);
        else
            printf("Could not save %s\n", idftFileName);

        // TODO: for modificiations maybe do lpf, hpf and frequency shift? maybe a command line option? nah, just do it, so you can drag a file on and get this stuff out
    }
    else
        printf("could not read 24 bit bmp file %s\n\n", srcFileName);

    return 0;
}

/*

First:
* generalize the progress % thing with a scoped object, that also tells you how long it took to run!  Also only erase / redraw percent if the percent has changed!
* probably should spit out phase of origional image, just for educational purposes

* don't make it spit out the grey scale image.  It's redundant with the basic idft image!
* make a diagonal striped image?
? try to shift the pixels up and right 1/2 of the bounds.
 * if that works, need to explain that on blog post!
* do logaraithmic transform!
* after it's working, make it separated on each axis so it's faster (check in what you have first!)
* make FFT work on greyscale converted image
* fft of images to understand them
 * a couple examples
* then ifft after frequency modifications?
 * shift frequencies
 * erase frequencies (high and low pass filtering)
 * change phase?
 * make it spit out how long it took, so can use it on blog


Blog:
 * note that we convert image to greyscale.  Otherwise we'd have to do it for each color channel.
  * http://blog.demofox.org/2014/02/03/converting-rgb-to-grayscale/
 * mention that FFT is separable, so you can do an axis at a time?
  * also that fast fourier transform is faster than naiive implementation.
  * also that it can be threaded (fork / join)
 * just brute force it for now
 * note how long it takes for various image sizes.
  * mention that it's O(4)
 * mention log transform of pixels to make things more visible
 * mention that phase is gotten via atan (imaginary / real)

Links:
 * http://homepages.inf.ed.ac.uk/rbf/HIPR2/fourier.htm
 * http://www.thefouriertransform.com/m/index.php
 * https://www.cs.unm.edu/~brayer/vision/fourier.html





Next:
* auto rotate images!
 * fft the image
 * do auto correlation with a rotating line (0-180 degrees)
 * rotate image, using bicubic sampling for interpolation
 * pad rotated image with black


Then: 
* DownScaleImage and notes there!

*/