#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <stdint.h>
#include <vector>
#include <complex>
#include <thread>
#include <atomic>
#include <random>

typedef uint8_t uint8;
typedef int64_t int64;

const float c_pi = 3.14159265359f;

//======================================================================================
struct SImageData
{
    SImageData ()
        : m_width(0)
        , m_height(0)
    { }
   
    size_t m_width;
    size_t m_height;
    size_t m_pitch;
    std::vector<uint8> m_pixels;
};

SImageData s_stippleImage;
 
//======================================================================================
struct SColor
{
    SColor (uint8 _R = 0, uint8 _G = 0, uint8 _B = 0)
        : R(_R), G(_G), B(_B)
    { }

    inline void Set (uint8 _R, uint8 _G, uint8 _B)
    {
        R = _R;
        G = _G;
        B = _B;
    }
 
    uint8 B, G, R;
};

//======================================================================================
struct SImageDataComplex
{
    SImageDataComplex ()
        : m_width(0)
        , m_height(0)
    { }
 
    size_t m_width;
    size_t m_height;
    std::vector<std::complex<float>> m_pixels;
};

//======================================================================================
std::complex<float> DFTPixel (const SImageData &srcImage, size_t K, size_t L)
{
    std::complex<float> ret(0.0f, 0.0f);
 
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
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
 
//======================================================================================
void DFTImage (const SImageData &srcImage, SImageDataComplex &destImage)
{
    // NOTE: this function assumes srcImage is greyscale, so works on only the red component of srcImage.
    // ImageToGrey() will convert an image to greyscale.

    // size the output dft data
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pixels.resize(destImage.m_width*destImage.m_height);

    size_t numThreads = std::thread::hardware_concurrency();
    //if (numThreads > 0)
        //numThreads = numThreads - 1;

    std::vector<std::thread> threads;
    threads.resize(numThreads);

    printf("Doing DFT with %zu threads...\n", numThreads);

    // calculate 2d dft (brute force, not using fast fourier transform) multithreadedly
    std::atomic<size_t> nextRow(0);
    for (std::thread& t : threads)
    {
        t = std::thread(
            [&] ()
            {
                size_t row = nextRow.fetch_add(1);
                bool reportProgress = (row == 0);
                int lastPercent = -1;

                while (row < srcImage.m_height)
                {
                    // calculate the DFT for every pixel / frequency in this row
                    for (size_t x = 0; x < srcImage.m_width; ++x)
                    {
                        destImage.m_pixels[row * destImage.m_width + x] = DFTPixel(srcImage, x, row);
                    }

                    // report progress if we should
                    if (reportProgress)
                    {
                        int percent = int(100.0f * float(row) / float(srcImage.m_height));
                        if (lastPercent != percent)
                        {
                            lastPercent = percent;
                            printf("            \rDFT: %i%%", lastPercent);
                        }
                    }

                    // go to the next row
                    row = nextRow.fetch_add(1);
                }
            }
        );
    }

    for (std::thread& t : threads)
        t.join();

    printf("\n");
}

//======================================================================================
void GetMagnitudeData (const SImageDataComplex& srcImage, SImageData& destImage)
{
    // size the output image
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pitch = 4 * ((srcImage.m_width * 24 + 31) / 32);
    destImage.m_pixels.resize(destImage.m_pitch*destImage.m_height);
 
    // get floating point magnitude data
    std::vector<float> magArray;
    magArray.resize(srcImage.m_width*srcImage.m_height);
    float maxmag = 0.0f;
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
        {
            // Offset the information by half width & height in the positive direction.
            // This makes frequency 0 (DC) be at the image origin, like most diagrams show it.
            int k = (x + (int)srcImage.m_width / 2) % (int)srcImage.m_width;
            int l = (y + (int)srcImage.m_height / 2) % (int)srcImage.m_height;
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
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
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
 
//======================================================================================
void GetPhaseData (const SImageDataComplex& srcImage, SImageData& destImage)
{
    // size the output image
    destImage.m_width = srcImage.m_width;
    destImage.m_height = srcImage.m_height;
    destImage.m_pitch = 4 * ((srcImage.m_width * 24 + 31) / 32);
    destImage.m_pixels.resize(destImage.m_pitch*destImage.m_height);
 
    // get floating point phase data, and encode it in [0,255]
    for (size_t x = 0; x < srcImage.m_width; ++x)
    {
        for (size_t y = 0; y < srcImage.m_height; ++y)
        {
            // Offset the information by half width & height in the positive direction.
            // This makes frequency 0 (DC) be at the image origin, like most diagrams show it.
            int k = (x + (int)srcImage.m_width / 2) % (int)srcImage.m_width;
            int l = (y + (int)srcImage.m_height / 2) % (int)srcImage.m_height;
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

//======================================================================================
bool ImageSave (const SImageData &image, const char *fileName)
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
    infoHeader.biWidth = (LONG)image.m_width;
    infoHeader.biHeight = (LONG)image.m_height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = (DWORD) image.m_pixels.size();
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

//======================================================================================
bool ImageLoad (const char *fileName, SImageData& imageData)
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
    imageData.m_pitch = 4 * ((imageData.m_width * 24 + 31) / 32);
 
    fclose(file);
    return true;
}

//======================================================================================
void ImageInit (SImageData& image, size_t width, size_t height)
{
    image.m_width = width;
    image.m_height = height;
    image.m_pitch = 4 * ((width * 24 + 31) / 32);
    image.m_pixels.resize(image.m_pitch * image.m_width);
    std::fill(image.m_pixels.begin(), image.m_pixels.end(), 0);
}

//======================================================================================
void SampleTest (const SImageData& image, const SImageData& samples, const char* fileName)
{
    SImageData outImage;
    ImageInit(outImage, image.m_width, image.m_height);

    for (size_t y = 0; y < image.m_height; ++y)
    {
        size_t sampleY = y % samples.m_height;
        for (size_t x = 0; x < image.m_width; ++x)
        {
            size_t sampleX = x % samples.m_width;

            const SColor* samplePixel = (SColor*)&samples.m_pixels[sampleY*samples.m_pitch + sampleX * 3];
            const SColor* imagePixel = (SColor*)&image.m_pixels[y*image.m_pitch + x * 3];

            SColor* outPixel = (SColor*)&outImage.m_pixels[y*outImage.m_pitch + x * 3];

            if (samplePixel->R == 255)
                *outPixel = *imagePixel;
        }
    }

    ImageSave(outImage, fileName);
}

//======================================================================================
int main (int argc, char** argv)
{
    const size_t c_imageSize = 256;
    const bool c_doDFT = false;

    // load the source image
    SImageData image;
    ImageLoad("Image.bmp", image);

    // white noise
    {
        const size_t samples1 = 100;
        const size_t samples2 = 1000;
        const size_t samples3 = 10000;

        SImageData samples;
        ImageInit(samples, c_imageSize, c_imageSize);

        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(0, c_imageSize-1);

        for (size_t i = 1; i <= samples3; ++i)
        {
            size_t x = dist(rng);
            size_t y = dist(rng);

            SColor* pixel = (SColor*)&samples.m_pixels[y*samples.m_pitch + x * 3];
            pixel->R = pixel->G = pixel->B = 255;

            if (i == samples1 || i == samples2 || i == samples3)
            {
                printf("White Noise %zu samples\n", i);

                char fileName[256];
                sprintf(fileName, "WhiteNoise_%zu.bmp", i);
                ImageSave(samples, fileName);

                sprintf(fileName, "WhiteNoise_%zu_samples.bmp", i);
                SampleTest(image, samples, fileName);

                if (c_doDFT)
                {
                    SImageDataComplex frequencyData;
                    DFTImage(samples, frequencyData);

                    SImageData magnitudeData;
                    GetMagnitudeData(frequencyData, magnitudeData);

                    sprintf(fileName, "WhiteNoise_%zu_mag.bmp", i);
                    ImageSave(magnitudeData, fileName);
                }
            }
        }
    }

    // regular samples
    {

        auto GridTest = [&] (size_t sampleCount) {
            SImageData samples;
            ImageInit(samples, c_imageSize, c_imageSize);

            size_t side = size_t(std::sqrt(float(sampleCount)));

            size_t pixels = c_imageSize / side;

            for (size_t y = 0; y < side; ++y)
            {
                size_t pixelY = y * pixels;
                for (size_t x = 0; x < side; ++x)
                {
                    size_t pixelX = x * pixels;

                    SColor* pixel = (SColor*)&samples.m_pixels[pixelY*samples.m_pitch + pixelX * 3];
                    pixel->R = pixel->G = pixel->B = 255;
                }
            }

            printf("Regular %zu samples\n", sampleCount);

            char fileName[256];
            sprintf(fileName, "Regular_%zu.bmp", sampleCount);
            ImageSave(samples, fileName);

            sprintf(fileName, "Regular_%zu_samples.bmp", sampleCount);
            SampleTest(image, samples, fileName);

            if (c_doDFT)
            {
                SImageDataComplex frequencyData;
                DFTImage(samples, frequencyData);

                SImageData magnitudeData;
                GetMagnitudeData(frequencyData, magnitudeData);

                sprintf(fileName, "Regular_%zu_mag.bmp", sampleCount);
                ImageSave(magnitudeData, fileName);
            }
        };

        GridTest(256);    // 16 * 16
        GridTest(1024);   // 32 * 32
        GridTest(16384);  // 128 * 128
    }

    return 0;
}

/*

TODO:
* blue noise
* make all the DFT's

*/