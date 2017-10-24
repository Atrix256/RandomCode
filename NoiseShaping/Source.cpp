#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers.  Sorry non windows people!
#include <stdint.h>
#include <vector>
#include <random>
#include <array>
#include <thread>
#include <complex>
#include <atomic>

typedef uint8_t uint8;
typedef int64_t int64;

const float c_pi = 3.14159265359f;

// settings
const size_t    c_imageSize = 256; // TODO: 256
const bool      c_doDFT = true; // TODO: true before checkin!
const float     c_blurThresholdPercent = 0.005f; // lower numbers give higher quality results, but take longer. This is 0.5%
const float     c_numBlurs = 5;

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
struct SImageDataFloat
{
    SImageDataFloat()
        : m_width(0)
        , m_height(0)
    { }
   
    size_t m_width;
    size_t m_height;
    std::vector<float> m_pixels;
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
void ImageDFT (const SImageData &srcImage, SImageDataComplex &destImage)
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
    image.m_pixels.resize(image.m_pitch * image.m_height);
    std::fill(image.m_pixels.begin(), image.m_pixels.end(), 0);
}

//======================================================================================
void ImageFloatInit (SImageDataFloat& image, size_t width, size_t height)
{
    image.m_width = width;
    image.m_height = height;
    image.m_pixels.resize(image.m_width * image.m_height);
    std::fill(image.m_pixels.begin(), image.m_pixels.end(), 0.0f);
}

//======================================================================================
int PixelsNeededForSigma (float sigma)
{
    // returns the number of pixels needed to represent a gaussian kernal that has values
    // down to the threshold amount.  A gaussian function technically has values everywhere
    // on the image, but the threshold lets us cut it off where the pixels contribute to
    // only small amounts that aren't as noticeable.
    return int(floor(1.0f + 2.0f * sqrtf(-2.0f * sigma * sigma * log(c_blurThresholdPercent)))) + 1;
}

//======================================================================================
float Gaussian (float sigma, float x)
{
    return expf(-(x*x) / (2.0f * sigma*sigma));
}

//======================================================================================
float GaussianSimpsonIntegration (float sigma, float a, float b)
{
    return
        ((b - a) / 6.0f) *
        (Gaussian(sigma, a) + 4.0f * Gaussian(sigma, (a + b) / 2.0f) + Gaussian(sigma, b));
}

//======================================================================================
std::vector<float> GaussianKernelIntegrals (float sigma, int taps)
{
    std::vector<float> ret;
    float total = 0.0f;
    for (int i = 0; i < taps; ++i)
    {
        float x = float(i) - float(taps / 2);
        float value = GaussianSimpsonIntegration(sigma, x - 0.5f, x + 0.5f);
        ret.push_back(value);
        total += value;
    }
    // normalize it
    for (unsigned int i = 0; i < ret.size(); ++i)
    {
        ret[i] /= total;
    }
    return ret;
}

//======================================================================================
const float* GetPixelWrapAround (const SImageDataFloat& image, int x, int y)
{
    if (x >= (int)image.m_width)
    {
        x = x % (int)image.m_width;
    }
    else
    {
        while (x < 0)
            x += (int)image.m_width;
    }

    if (y >= (int)image.m_height)
    {
        y = y % (int)image.m_height;
    }
    else
    {
        while (y < 0)
            y += (int)image.m_height;
    }

    return &image.m_pixels[(y * image.m_width) + x];
}

//======================================================================================
void ImageGaussianBlur (const SImageDataFloat& srcImage, SImageDataFloat &destImage, float xblursigma, float yblursigma, unsigned int xblursize, unsigned int yblursize)
{
    // allocate space for copying the image for destImage and tmpImage
    ImageFloatInit(destImage, srcImage.m_width, srcImage.m_height);
 
    SImageDataFloat tmpImage;
    ImageFloatInit(tmpImage, srcImage.m_width, srcImage.m_height);
 
    // horizontal blur from srcImage into tmpImage
    {
        auto row = GaussianKernelIntegrals(xblursigma, xblursize);
 
        int startOffset = -1 * int(row.size() / 2);
 
        for (int y = 0; y < tmpImage.m_height; ++y)
        {
            for (int x = 0; x < tmpImage.m_width; ++x)
            {
                float blurredPixel = 0.0f;
                for (unsigned int i = 0; i < row.size(); ++i)
                {
                    const float *pixel = GetPixelWrapAround(srcImage, x + startOffset + i, y);
                    blurredPixel += pixel[0] * row[i];
                }
                 
                float *destPixel = &tmpImage.m_pixels[y * tmpImage.m_width + x];
                destPixel[0] = blurredPixel;
            }
        }
    }
 
    // vertical blur from tmpImage into destImage
    {
        auto row = GaussianKernelIntegrals(yblursigma, yblursize);
 
        int startOffset = -1 * int(row.size() / 2);
 
        for (int y = 0; y < destImage.m_height; ++y)
        {
            for (int x = 0; x < destImage.m_width; ++x)
            {
                float blurredPixel = 0.0f;
                for (unsigned int i = 0; i < row.size(); ++i)
                {
                    const float *pixel = GetPixelWrapAround(tmpImage, x, y + startOffset + i);
                    blurredPixel += pixel[0] * row[i];
                }
 
                float *destPixel = &destImage.m_pixels[y * destImage.m_width + x];
                destPixel[0] = blurredPixel;
            }
        }
    }
}

//======================================================================================
void SaveImageFloatAsBMP (const SImageDataFloat& imageFloat, const char* fileName)
{
    printf("\n%s\n", fileName);

    // init the image
    SImageData image;
    ImageInit(image, imageFloat.m_width, imageFloat.m_height);

    // write the data to the image
    const float* srcData = &imageFloat.m_pixels[0];
    for (size_t y = 0; y < image.m_height; ++y)
    {
        SColor* pixel = (SColor*)&image.m_pixels[y*image.m_pitch];

        for (size_t x = 0; x < image.m_width; ++x)
        {
            uint8 value = uint8(255.0f * srcData[0]);

            pixel->Set(value, value, value);

            ++pixel;
            ++srcData;
        }
    }

    // save the image
    ImageSave(image, fileName);

    // also save a DFT of the image
    if (c_doDFT)
    {
        SImageDataComplex dftData;
        ImageDFT(image, dftData);

        SImageData DFTMagImage;
        GetMagnitudeData(dftData, DFTMagImage);

        char buffer[256];
        sprintf(buffer, "%s.mag.bmp", fileName);

        ImageSave(DFTMagImage, buffer);
    }
}

//======================================================================================
void NormalizeHistogram (SImageDataFloat& image)
{
    struct SHistogramHelper
    {
        float value;
        size_t pixelIndex;
    };
    static std::vector<SHistogramHelper> pixels;
    pixels.resize(image.m_width * image.m_height);

    // put all the pixels into the array
    for (size_t i = 0, c = image.m_width * image.m_height; i < c; ++i)
    {
        pixels[i].value = image.m_pixels[i];
        pixels[i].pixelIndex = i;
    }

    // shuffle the pixels to randomly order ties. not as big a deal with floating point pixel values though
    static std::random_device rd;
    static std::mt19937 rng(rd());
    std::shuffle(pixels.begin(), pixels.end(), rng);

    // sort the pixels by value
    std::sort(
        pixels.begin(),
        pixels.end(),
        [] (const SHistogramHelper& a, const SHistogramHelper& b)
        {
            return a.value < b.value;
        }
    );

    // use the pixel's place in the array as the new value, and write it back to the image
    for (size_t i = 0, c = image.m_width * image.m_height; i < c; ++i)
    {
        float value = float(i) / float(c - 1);
        image.m_pixels[pixels[i].pixelIndex] = value;
    }
}

//======================================================================================
void BlueNoiseTest (float blurSigma)
{
    // calculate the blur size from our sigma
    int blurSize = PixelsNeededForSigma(blurSigma) | 1;

    // setup the randon number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // generate some white noise
    SImageDataFloat noise;
    ImageFloatInit(noise, c_imageSize, c_imageSize);
    for (float& v : noise.m_pixels)
    {
        v = dist(rng);
    }

    // save off the starting white noise
    const char* baseFileName = "bluenoise_%i_%zu.bmp";
    char fileName[256];

    sprintf(fileName, baseFileName, int(blurSigma * 100.0f), 0);
    SaveImageFloatAsBMP(noise, fileName);

    // iteratively high pass filter and rescale histogram to the 0 to 1 range
    SImageDataFloat blurredImage;
    for (size_t blurIndex = 0; blurIndex < c_numBlurs; ++blurIndex)
    {
        // get a low passed version of the current image
        ImageGaussianBlur(noise, blurredImage, blurSigma, blurSigma, blurSize, blurSize);

        // subtract the low passed version to get the high passed version
        for (size_t pixelIndex = 0; pixelIndex < c_imageSize * c_imageSize; ++pixelIndex)
            noise.m_pixels[pixelIndex] -= blurredImage.m_pixels[pixelIndex];

        // put all pixels between 0.0 and 1.0 again
        NormalizeHistogram(noise);

        // save this image
        sprintf(fileName, baseFileName, int(blurSigma * 100.0f), blurIndex + 1);
        SaveImageFloatAsBMP(noise, fileName);
    }
}

//======================================================================================
void RedNoiseTest (float blurSigma)
{
    // calculate the blur size from our sigma
    int blurSize = PixelsNeededForSigma(blurSigma) | 1;

    // setup the randon number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // generate some white noise
    SImageDataFloat noise;
    ImageFloatInit(noise, c_imageSize, c_imageSize);
    for (float& v : noise.m_pixels)
    {
        v = dist(rng);
    }

    // save off the starting white noise
    const char* baseFileName = "rednoise_%i_%zu.bmp";
    char fileName[256];

    sprintf(fileName, baseFileName, int(blurSigma * 100.0f), 0);
    SaveImageFloatAsBMP(noise, fileName);

    // iteratively high pass filter and rescale histogram to the 0 to 1 range
    SImageDataFloat blurredImage;
    for (size_t blurIndex = 0; blurIndex < c_numBlurs; ++blurIndex)
    {
        // get a low passed version of the current image
        ImageGaussianBlur(noise, blurredImage, blurSigma, blurSigma, blurSize, blurSize);

        // set noise image to the low passed version
        noise.m_pixels = blurredImage.m_pixels;

        // put all pixels between 0.0 and 1.0 again
        NormalizeHistogram(noise);

        // save this image
        sprintf(fileName, baseFileName, int(blurSigma * 100.0f), blurIndex + 1);
        SaveImageFloatAsBMP(noise, fileName);
    }
}

//======================================================================================
void BandPassTest (float blurSigma1, float blurSigma2)
{
    // calculate the blur size from our sigma
    int blurSize1 = PixelsNeededForSigma(blurSigma1) | 1;
    int blurSize2 = PixelsNeededForSigma(blurSigma2) | 1;

    // setup the randon number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // generate some white noise
    SImageDataFloat noise;
    ImageFloatInit(noise, c_imageSize, c_imageSize);
    for (float& v : noise.m_pixels)
    {
        v = dist(rng);
    }

    // save off the starting white noise
    const char* baseFileName = "bandpass_%i_%i_%zu.bmp";
    char fileName[256];

    sprintf(fileName, baseFileName, int(blurSigma1 * 100.0f), int(blurSigma2 * 100.0f), 0);
    SaveImageFloatAsBMP(noise, fileName);

    // iteratively high pass filter and rescale histogram to the 0 to 1 range
    SImageDataFloat blurredImage1;
    SImageDataFloat blurredImage2;
    for (size_t blurIndex = 0; blurIndex < c_numBlurs; ++blurIndex)
    {
        // get two low passed versions of the current image
        ImageGaussianBlur(noise, blurredImage1, blurSigma1, blurSigma1, blurSize1, blurSize1);
        ImageGaussianBlur(noise, blurredImage2, blurSigma2, blurSigma2, blurSize2, blurSize2);

        // subtract one low passed version from the other
        for (size_t pixelIndex = 0; pixelIndex < c_imageSize * c_imageSize; ++pixelIndex)
            noise.m_pixels[pixelIndex] = blurredImage1.m_pixels[pixelIndex] - blurredImage2.m_pixels[pixelIndex];

        // put all pixels between 0.0 and 1.0 again
        NormalizeHistogram(noise);

        // save this image
        sprintf(fileName, baseFileName, int(blurSigma1 * 100.0f), int(blurSigma2 * 100.0f), blurIndex + 1);
        SaveImageFloatAsBMP(noise, fileName);
    }
}

//======================================================================================
void BandStopTest (float blurSigma1, float blurSigma2)
{
    // calculate the blur size from our sigma
    int blurSize1 = PixelsNeededForSigma(blurSigma1) | 1;
    int blurSize2 = PixelsNeededForSigma(blurSigma2) | 1;

    // setup the randon number generator
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // generate some white noise
    SImageDataFloat noise;
    ImageFloatInit(noise, c_imageSize, c_imageSize);
    for (float& v : noise.m_pixels)
    {
        v = dist(rng);
    }

    // save off the starting white noise
    const char* baseFileName = "bandstop_%i_%i_%zu.bmp";
    char fileName[256];

    sprintf(fileName, baseFileName, int(blurSigma1 * 100.0f), int(blurSigma2 * 100.0f), 0);
    SaveImageFloatAsBMP(noise, fileName);

    // iteratively high pass filter and rescale histogram to the 0 to 1 range
    SImageDataFloat blurredImage1;
    SImageDataFloat blurredImage2;
    for (size_t blurIndex = 0; blurIndex < c_numBlurs; ++blurIndex)
    {
        // get two low passed versions of the current image
        ImageGaussianBlur(noise, blurredImage1, blurSigma1, blurSigma1, blurSize1, blurSize1);
        ImageGaussianBlur(noise, blurredImage2, blurSigma2, blurSigma2, blurSize2, blurSize2);

        // subtract one low passed version from the other to get the pandpass noise, and subtract that from the original noise to get the band stop noise
        for (size_t pixelIndex = 0; pixelIndex < c_imageSize * c_imageSize; ++pixelIndex)
            noise.m_pixels[pixelIndex] -= (blurredImage1.m_pixels[pixelIndex] - blurredImage2.m_pixels[pixelIndex]);

        // put all pixels between 0.0 and 1.0 again
        NormalizeHistogram(noise);

        // save this image
        sprintf(fileName, baseFileName, int(blurSigma1 * 100.0f), int(blurSigma2 * 100.0f), blurIndex + 1);
        SaveImageFloatAsBMP(noise, fileName);
    }
}

//======================================================================================
int main (int argc, char ** argv)
{
    BlueNoiseTest(0.5f);
    BlueNoiseTest(1.0f);
    BlueNoiseTest(2.0f);

    RedNoiseTest(0.5f);
    RedNoiseTest(1.0f);
    RedNoiseTest(2.0f);

    BandPassTest(0.5f, 2.0f);

    BandStopTest(0.5f, 2.0f);

    return 0;
}

/*

TODO:

* animated gifs showing the noise and their DFT evolve?

* maybe also animate blue noise w/ golden ratio, and mention that in this post? vs a flip book of blue noise?

* TODOs in code

? how to demo the quality of this noise?
 * compare image and DFT to blue noise image and DFT
 * show thresholding (animation?) of noise textures
 * some kind of actual sampling thing or something?
 * show whether noise tiles
 * also just use it for greyscale image dithering?

* links
 * https://bartwronski.com/2016/10/30/dithering-part-two-golden-ratio-sequence-blue-noise-and-highpass-and-remap/
 * https://gpuopen.com/vdr-follow-up-fine-art-of-film-grain/
 * gaussian blur: https://blog.demofox.org/2015/08/19/gaussian-blur/
 * https://www.solidangle.com/research/dither_abstract.pdf

Blog:
* digital alchemy? turning white noise into other noise
! let's make some noise gif
* note that blur needs to wrap around!
* note: you can't make blue noise by making white noise, doing DFT, modifying stuff, then doing IDFT. that is filtering it and is equivelant to what you are doing here.
* note: timothy lottes says to use sort that's more efficient for fixed sizes keys (radix sort), and fits it in 64 bits instead of a struct.
* Note: likely want a better algorithm if doing offline. But this algorithm is pretty easy

* different blurs are different quality low pass filters.
 * box blur = low quality
 * gaussian blur = better.
 * best = sync.

*/