#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!
#include <vector>
#include <stdint.h>
#include <random>

typedef uint8_t uint8;

#define NUM_SAMPLES 100

#define IMAGE_WIDTH 600
#define IMAGE1D_HEIGHT 50
#define IMAGE_PAD   30

#define IMAGE_CENTERX ((IMAGE_WIDTH+IMAGE_PAD*2)/2)
#define IMAGE1D_CENTERY ((IMAGE1D_HEIGHT+IMAGE_PAD*2)/2)

#define AXIS_HEIGHT 40
#define DATA_HEIGHT 20
#define DATA_WIDTH 2

#define COLOR_FILL SColor(255,255,255)
#define COLOR_AXIS SColor(0, 0, 0)
#define COLOR_DATAPOINT SColor(255, 0, 0)

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

struct SColor
{
	SColor (uint8 _R = 0, uint8 _G = 0, uint8 _B = 0)
		: R(_R), G(_G), B(_B)
	{ }

	uint8 B, G, R;
};

//======================================================================================
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
 
    printf("%s saved\n", fileName);
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
void ImageClear (SImageData& image, const SColor& color)
{
	uint8* row = &image.m_pixels[0];
	for (size_t rowIndex = 0; rowIndex < image.m_height; ++rowIndex)
	{
		SColor* pixels = (SColor*)row;
		std::fill(pixels, pixels + image.m_width, color);

		row += image.m_pitch;
	}
}

//======================================================================================
void ImageBox (SImageData& image, size_t x1, size_t x2, size_t y1, size_t y2, const SColor& color)
{
	for (size_t y = y1; y < y2; ++y)
	{
		uint8* row = &image.m_pixels[y * image.m_pitch];
		SColor* start = &((SColor*)row)[x1];
		std::fill(start, start + x2 - x1, color);
	}
}

//======================================================================================
void DrawDecorations1DPre (SImageData& image)
{
    ImageClear(image, COLOR_FILL);
}

//======================================================================================
void DrawDecorations1DPost (SImageData& image)
{
    // draw the axes lines

    // Horizontal line
    ImageBox(image, IMAGE_PAD, IMAGE_WIDTH + IMAGE_PAD, IMAGE1D_CENTERY, IMAGE1D_CENTERY + 1, COLOR_AXIS);

    // vertical lines
    ImageBox(image, IMAGE_PAD, IMAGE_PAD + 1, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);
    ImageBox(image, IMAGE_WIDTH + IMAGE_PAD, IMAGE_WIDTH + IMAGE_PAD + 1, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);
}

//======================================================================================
SColor DataPointColor (size_t sampleIndex, size_t sampleCount)
{
    SColor ret;
    float percent = (float(sampleIndex) / (float(sampleCount) - 1.0f));

    ret.R = uint8((1.0f - percent) * 255.0f);
    ret.G = 0;
    ret.B = uint8(percent * 255.0f);

    float mag = (float)sqrt(ret.R*ret.R + ret.G*ret.G + ret.B*ret.B);
    ret.R = uint8((float(ret.R) / mag)*255.0f);
    ret.G = uint8((float(ret.G) / mag)*255.0f);
    ret.B = uint8((float(ret.B) / mag)*255.0f);

    return ret;
}

//======================================================================================
float RandomFloat (float min, float max)
{
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt);
}

//======================================================================================
template<bool JITTER>
void TestUniform1D ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorations1DPre(image);

	// draw the sample points
    const float c_halfJitter = 1.0f / float((NUM_SAMPLES + 1) * 2);
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
		float sample = float(i+1) / (float(NUM_SAMPLES)+1.0f);

        if (JITTER)
            sample += RandomFloat(-c_halfJitter, c_halfJitter);

		size_t pos = size_t(sample * float(IMAGE_WIDTH)) + IMAGE_PAD;
		ImageBox(image, pos, pos+1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i, NUM_SAMPLES));
	}

    // draw everything else
    DrawDecorations1DPost(image);

	// save the image
    if (JITTER)
	    SaveImage("1DUniformJitter.bmp", image);
    else
        SaveImage("1DUniform.bmp", image);
}

//======================================================================================
void TestUniformRandom1D ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);

    // setup the canvas
    DrawDecorations1DPre(image);

	// draw the sample points
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
        float sample = RandomFloat(0.0f, 1.0f);
		size_t pos = size_t(sample * float(IMAGE_WIDTH)) + IMAGE_PAD;
        ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i, NUM_SAMPLES));
	}

    // draw everything else
    DrawDecorations1DPost(image);

	// save the image
	SaveImage("1DUniformRandom.bmp", image);
}

//======================================================================================
template <size_t BASE, bool JITTER>
void TestVanDerCorput ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorations1DPre(image);

	// draw the sample points
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
        float sample = 0.0f;
        float denominator = float(BASE);
        size_t n = i;
        while (n > 0)
        {
            size_t multiplier = n % BASE;
            sample += float(multiplier) / denominator;
            n = n / BASE;
            denominator *= BASE;
        }

        if (JITTER)
        {
            float halfJitter = BASE;
            size_t n = i / BASE;
            while (n > 0)
            {
                halfJitter *= BASE;
                n /= BASE;
            }
            halfJitter = 1.0f / (halfJitter * 2.0f);

            sample += RandomFloat(-halfJitter, halfJitter);
        }

		size_t pos = size_t(sample * float(IMAGE_WIDTH)) + IMAGE_PAD;
        ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i, NUM_SAMPLES));
	}

    // draw everything else
    DrawDecorations1DPost(image);

	// save the image
    char fileName[256];
    if (JITTER)
        sprintf(fileName, "1DVanDerCorputJitter_%zu.bmp", BASE);
    else
        sprintf(fileName, "1DVanDerCorput_%zu.bmp", BASE);
	SaveImage(fileName, image);
}

//======================================================================================
void TestGoldenRatio ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorations1DPre(image);

	// draw the sample points
    const float c_goldenRatioConugate = 0.61803398875f;
    float sample = 0.0f;
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
        sample = std::fmodf(sample + c_goldenRatioConugate, 1.0f);
		size_t pos = size_t(sample * float(IMAGE_WIDTH)) + IMAGE_PAD;
        ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i, NUM_SAMPLES));
	}

    // draw everything else
    DrawDecorations1DPost(image);

	// save the image
	SaveImage("1DGoldenRatio.bmp", image);
}

//======================================================================================
int main (int argc, char **argv)
{
    // 1D tests
    {
        TestUniform1D<false>();
        TestUniform1D<true>();

        TestUniformRandom1D();

        TestVanDerCorput<2, false>();
        TestVanDerCorput<3, false>();
        TestVanDerCorput<4, false>();

        // TODO: try halving jitter amount maybe, or lower? or constant, small jitter amount?
        TestVanDerCorput<2, true>();
        TestVanDerCorput<3, true>();
        TestVanDerCorput<4, true>();

        // TODO: jitter!
        TestGoldenRatio();
    }
}

/*

TODO:

* color points based on sample number!

* do fft for both 1d and 2d? does it make sense for sample points?
 * if not, how do we analyze quality?
 * maybe we calculate variance? i dunno...

* 1d
 * jitter uniform
 * jitter van der corput
 * jitter golden ratio
 ? does hammersly come in 1d? I think so!
 * ??

? sobol sequence?

* 2d
 * halton sequence
 * Hammersly
 * uniform (grid)
 * uniform random
 ? jitter all 2d versions?
 ? does fibanoci come in 2d?
 * ??

* van der corput based shuffling.
 * can seed (kind of) by changing base
 * 2d shuffle?

? count the bits to make white noise into gaussian? or save this for another post?

LINKS:
fibanocci colors: http://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/

https://en.m.wikipedia.org/wiki/Halton_sequence

https://en.m.wikipedia.org/wiki/Van_der_Corput_sequence

grey codes

format preserving encryption post

blue noise posts:
    http://momentsingraphics.de/?p=127
    http://momentsingraphics.de/?p=148
    http://www.joesfer.com/?p=108


*
 
*/