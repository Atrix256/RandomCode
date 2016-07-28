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
 
void DFTImage (const SImageData &srcImage, SImageDataComplex &destImage)
{
    // NOTE: this function assumes srcImage is greyscale, so works on only the red component of srcImage.
    // ImageToGrey() will convert an image to greyscale.

    // size the output dft data
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pixels.resize(destImage.m_width*destImage.m_height);

    SProgress progress("DFT:", srcImage.m_width * srcImage.m_height);
 
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
}

uint8 InverseDFTPixel (const SImageDataComplex &srcImage, int K, int L)
{
    std::complex<float> total(0.0f, 0.0f);
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // Get the pixel value
            const std::complex<float> &src = srcImage.m_pixels[(y * srcImage.m_width) + x];

            // Add to the sum of the return value
            float v = float(K * x) / float(srcImage.m_width);
            v += float(L * y) / float(srcImage.m_height);
            std::complex<float> result = src * std::polar<float>(1.0f, 2.0f * c_pi * v);

            // sum up the results
            total += result;
        }
    }

    float idft = std::abs(total) / float(srcImage.m_width*srcImage.m_height);

    // make sure the values are in range
    if (idft < 0.0f)
        idft = 0.0f;
    if (idft > 1.0f)
        idft = 1.0;

    return uint8(idft * 255.0f);
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

    SProgress progress("Inverse DFT:", srcImage.m_width*srcImage.m_height);

    // calculate inverse 2d dft (brute force, not using fast fourier transform)
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // calculate DFT for that pixel / frequency
            uint8 idft = InverseDFTPixel(srcImage, x, y);
            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = idft;
            dest[1] = idft;
            dest[2] = idft;

            // update progress
            progress.Update();
        }
    }
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

void GetPhaseData (const SImageDataComplex& srcImage, SImageData& destImage)
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

    // get floating point phase data, and encode it in [0,255]
    for (int x = 0; x < srcImage.m_width; ++x)
    {
        for (int y = 0; y < srcImage.m_height; ++y)
        {
            // Offset the information by half width & height in the positive direction.
            // This makes frequency 0 (DC) be at the image origin, like most diagrams show it.
            int k = (x + srcImage.m_width / 2) % srcImage.m_width;
            int l = (y + srcImage.m_height / 2) % srcImage.m_height;
            const std::complex<float> &src = srcImage.m_pixels[l*srcImage.m_width + k];

            // get phase, and change it from [-pi,+pi] to [0,255]
            float phase = (0.5f + 0.5f * std::atan2(src.real(), src.imag()) / c_pi);
            if (phase < 0.0f)
                phase = 0.0f;
            if (phase > 1.0f)
                phase = 1.0;
            uint8 phase255 = uint8(phase * 255);

            // write the phase as grey scale color
            uint8* dest = &destImage.m_pixels[y*destImage.m_pitch + x * 3];
            dest[0] = phase255;
            dest[1] = phase255;
            dest[2] = phase255;
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
    if (LoadImage(srcFileName, srcImage))
    {
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
            strcat(outFileName, ".raw.mag.bmp");

            SImageData destImage;
            GetMagnitudeData(frequencyData, destImage);
            SaveImage(outFileName, destImage);
        }

        // save phase information
        {
            char outFileName[1024];
            strcpy(outFileName, baseFileName);
            strcat(outFileName, ".raw.phase.bmp");

            SImageData destImage;
            GetPhaseData(frequencyData, destImage);
            SaveImage(outFileName, destImage);
        }

        // inverse dft the modified frequency and save the result
        {
            char outFileName[1024];
            strcpy(outFileName, baseFileName);
            strcat(outFileName, ".raw.idft.bmp");

            SImageData modifiedImage;
            InverseDFTImage(frequencyData, modifiedImage);
            SaveImage(outFileName, modifiedImage);
        }

        // Low Pass Filter: Remove high frequencies, write out frequency magnitudes, write out inverse dft
        {
            printf("\n=====LPF=====\n");

            // remove frequencies that are too far from frequency 0.
            // Note that even though our output frequency images have frequency 0 (DC) in the center, that
            // isn't actually how it's stored in our SImageDataComplex structure.  Pixel (0,0) is frequency 0.
            SImageDataComplex dft = frequencyData;
            float halfWidth = float(dft.m_width / 2);
            float halfHeight = float(dft.m_height / 2);
            for (int x = 0; x < dft.m_width; ++x)
            {
                for (int y = 0; y < dft.m_height; ++y)
                {
                    float relX = 0.0f;
                    float relY = 0.0f;

                    if (x < halfWidth)
                        relX = float(x) / halfWidth;
                    else
                        relX = (float(x) - float(dft.m_width)) / halfWidth;

                    if (y < halfHeight)
                        relY = float(y) / halfHeight;
                    else
                        relY = (float(y) - float(dft.m_height)) / halfHeight;

                    float dist = sqrt(relX*relX + relY*relY) / c_rootTwo; // divided by root 2 so our distance is from 0 to 1
                    if (dist > 0.1f)
                        dft.m_pixels[y*dft.m_width + x] = std::complex<float>(0.0f, 0.0f);
                }
            }

            // write dft magnitude data
            char outFileName[1024];
            strcpy(outFileName, baseFileName);
            strcat(outFileName, ".lpf.mag.bmp");
            SImageData destImage;
            GetMagnitudeData(dft, destImage);
            SaveImage(outFileName, destImage);

            // inverse dft and save the image
            strcpy(outFileName, baseFileName);
            strcat(outFileName, ".lpf.idft.bmp");
            SImageData modifiedImage;
            InverseDFTImage(dft, modifiedImage);
            SaveImage(outFileName, modifiedImage);
        }

        // High Pass Filter: Remove low frequencies, write out frequency magnitudes, write out inverse dft
        {
            printf("\n=====HPF=====\n");

            // remove frequencies that are too close to frequency 0.
            // Note that even though our output frequency images have frequency 0 (DC) in the center, that
            // isn't actually how it's stored in our SImageDataComplex structure.  Pixel (0,0) is frequency 0.
            SImageDataComplex dft = frequencyData;
            float halfWidth = float(dft.m_width / 2);
            float halfHeight = float(dft.m_height / 2);
            for (int x = 0; x < dft.m_width; ++x)
            {
                for (int y = 0; y < dft.m_height; ++y)
                {
                    float relX = 0.0f;
                    float relY = 0.0f;

                    if (x < halfWidth)
                        relX = float(x) / halfWidth;
                    else
                        relX = (float(x) - float(dft.m_width)) / halfWidth;

                    if (y < halfHeight)
                        relY = float(y) / halfHeight;
                    else
                        relY = (float(y) - float(dft.m_height)) / halfHeight;

                    float dist = sqrt(relX*relX + relY*relY) / c_rootTwo; // divided by root 2 so our distance is from 0 to 1
                    if (dist < 0.1f)
                        dft.m_pixels[y*dft.m_width + x] = std::complex<float>(0.0f, 0.0f);
                }
            }

            // write dft magnitude data
            char outFileName[1024];
            strcpy(outFileName, baseFileName);
            strcat(outFileName, ".hpf.mag.bmp");
            SImageData destImage;
            GetMagnitudeData(dft, destImage);
            SaveImage(outFileName, destImage);

            // inverse dft and save the image
            strcpy(outFileName, baseFileName);
            strcat(outFileName, ".hpf.idft.bmp");
            SImageData modifiedImage;
            InverseDFTImage(dft, modifiedImage);
            SaveImage(outFileName, modifiedImage);
        }

        // ZeroPhase
        {
            printf("\n=====Zero Phase=====\n");

            // Set phase to zero for all frequencies.
            // Note that even though our output frequency images have frequency 0 (DC) in the center, that
            // isn't actually how it's stored in our SImageDataComplex structure.  Pixel (0,0) is frequency 0.
            SImageDataComplex dft = frequencyData;
            float halfWidth = float(dft.m_width / 2);
            float halfHeight = float(dft.m_height / 2);
            for (int x = 0; x < dft.m_width; ++x)
            {
                for (int y = 0; y < dft.m_height; ++y)
                {
                    std::complex<float>& v = dft.m_pixels[y*dft.m_width + x];
                    float mag = std::abs(v);
                    v = std::complex<float>(mag, 0.0f);
                }
            }

            // write dft magnitude data
            char outFileName[1024];
            strcpy(outFileName, baseFileName);
            strcat(outFileName, ".phase0.mag.bmp");
            SImageData destImage;
            GetMagnitudeData(dft, destImage);
            SaveImage(outFileName, destImage);

            // inverse dft and save the image
            strcpy(outFileName, baseFileName);
            strcat(outFileName, ".phase0.idft.bmp");
            SImageData modifiedImage;
            InverseDFTImage(dft, modifiedImage);
            SaveImage(outFileName, modifiedImage);
        }
    }
    else
        printf("could not read 24 bit bmp file %s\n\n", srcFileName);

    return 0;
}

/*

Blog:
 * note that we convert image to greyscale.  Otherwise we'd have to do it for each color channel.
  * http://blog.demofox.org/2014/02/03/converting-rgb-to-grayscale/
 * mention that FFT is separable, so you can do an axis at a time?
  * also that fast fourier transform is faster than naiive implementation.
  * also that it can be threaded (fork / join)
 * just brute force it for now
 * note how long it takes for various image sizes.
  * mention that it's O(4)
 * mention that magnitude is just the length of the vector
 * mention log transform of pixels to make things more visible
 * mention that phase is gotten via atan (imaginary / real)
 * low frequencies are in middle, high frequencies are at the edges
 * note how the zelda guy gets what looks like compression artifacts.  Some compression schemes work by removing frequencies.
 * phase matters a bunch for images.  In sound, your ears cant detect changes in phase that well though!

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