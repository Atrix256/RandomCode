#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!
#include <vector>
#include <stdint.h>
#include <random>

typedef uint8_t uint8;

// TODO: put this back to 11
#define NUM_SAMPLES 64
#define NUM_SAMPLES_FOR_COLORING 100

#define IMAGE1D_WIDTH 600
#define IMAGE1D_HEIGHT 50
#define IMAGE2D_WIDTH 300
#define IMAGE2D_HEIGHT 300
#define IMAGE_PAD   30

#define IMAGE1D_CENTERX ((IMAGE1D_WIDTH+IMAGE_PAD*2)/2)
#define IMAGE1D_CENTERY ((IMAGE1D_HEIGHT+IMAGE_PAD*2)/2)
#define IMAGE2D_CENTERX ((IMAGE2D_WIDTH+IMAGE_PAD*2)/2)
#define IMAGE2D_CENTERY ((IMAGE2D_HEIGHT+IMAGE_PAD*2)/2)

#define AXIS_HEIGHT 40
#define DATA_HEIGHT 20
#define DATA_WIDTH 2

#define COLOR_FILL SColor(255,255,255)
#define COLOR_AXIS SColor(0, 0, 0)

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
void DrawDecorationsPre (SImageData& image)
{
    ImageClear(image, COLOR_FILL);
}

//======================================================================================
void DrawDecorations1DPost (SImageData& image)
{
    // draw the axes lines

    // Horizontal line
    ImageBox(image, IMAGE_PAD, IMAGE1D_WIDTH + IMAGE_PAD, IMAGE1D_CENTERY, IMAGE1D_CENTERY + 1, COLOR_AXIS);

    // vertical lines
    ImageBox(image, IMAGE_PAD, IMAGE_PAD + 1, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);
    ImageBox(image, IMAGE1D_WIDTH + IMAGE_PAD, IMAGE1D_WIDTH + IMAGE_PAD + 1, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);
}

//======================================================================================
void DrawDecorations2DPost (SImageData& image)
{
    // draw a box

    // horizontal lines
    ImageBox(image, IMAGE_PAD - 1, IMAGE2D_WIDTH + IMAGE_PAD + 1, IMAGE_PAD - 1, IMAGE_PAD, COLOR_AXIS);
    ImageBox(image, IMAGE_PAD - 1, IMAGE2D_WIDTH + IMAGE_PAD + 1, IMAGE2D_HEIGHT + IMAGE_PAD, IMAGE2D_HEIGHT + IMAGE_PAD + 1, COLOR_AXIS);

    // vertical lines
    ImageBox(image, IMAGE_PAD - 1, IMAGE_PAD, IMAGE_PAD - 1, IMAGE2D_HEIGHT + IMAGE_PAD + 1, COLOR_AXIS);
    ImageBox(image, IMAGE_PAD + IMAGE2D_WIDTH, IMAGE_PAD + IMAGE2D_WIDTH + 1, IMAGE_PAD - 1, IMAGE2D_HEIGHT + IMAGE_PAD + 1, COLOR_AXIS);
}

//======================================================================================
SColor DataPointColor (size_t sampleIndex)
{
    SColor ret;
    float percent = (float(sampleIndex) / (float(NUM_SAMPLES_FOR_COLORING) - 1.0f));

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
size_t Ruler (size_t n)
{
	size_t ret = 0;
	while (n != 0 && (n & 1) == 0)
	{
		n /= 2;
		++ret;
	}
	return ret;
}

//======================================================================================
void TestUniform1D (bool jitter)
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE1D_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorationsPre(image);

	// draw the sample points
    const float c_halfJitter = 1.0f / float((NUM_SAMPLES + 1) * 2);
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
		float sample = float(i+1) / (float(NUM_SAMPLES)+1.0f);

        if (jitter)
            sample += RandomFloat(-c_halfJitter, c_halfJitter);

		size_t pos = size_t(sample * float(IMAGE1D_WIDTH)) + IMAGE_PAD;
		ImageBox(image, pos, pos+1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i));
	}

    // draw everything else
    DrawDecorations1DPost(image);

	// save the image
    if (jitter)
	    SaveImage("1DUniformJitter.bmp", image);
    else
        SaveImage("1DUniform.bmp", image);
}

//======================================================================================
void TestUniformRandom1D ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE1D_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);

    // setup the canvas
    DrawDecorationsPre(image);

	// draw the sample points
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
        float sample = RandomFloat(0.0f, 1.0f);
		size_t pos = size_t(sample * float(IMAGE1D_WIDTH)) + IMAGE_PAD;
        ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i));
	}

    // draw everything else
    DrawDecorations1DPost(image);

	// save the image
	SaveImage("1DUniformRandom.bmp", image);
}

//======================================================================================
void TestVanDerCorput (size_t base)
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE1D_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorationsPre(image);

	// draw the sample points
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
        float sample = 0.0f;
        float denominator = float(base);
        size_t n = i;
        while (n > 0)
        {
            size_t multiplier = n % base;
            sample += float(multiplier) / denominator;
            n = n / base;
            denominator *= base;
        }

		size_t pos = size_t(sample * float(IMAGE1D_WIDTH)) + IMAGE_PAD;
        ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i));
	}

    // draw everything else
    DrawDecorations1DPost(image);

	// save the image
    char fileName[256];
	sprintf(fileName, "1DVanDerCorput_%zu.bmp", base);
	SaveImage(fileName, image);
}

//======================================================================================
void TestGoldenRatio (float seed)
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE1D_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorationsPre(image);

	// draw the sample points
    const float c_goldenRatioConugate = 0.61803398875f;
    float sample = seed;
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
        sample = std::fmodf(sample + c_goldenRatioConugate, 1.0f);
		size_t pos = size_t(sample * float(IMAGE1D_WIDTH)) + IMAGE_PAD;
        ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i));
	}

    // draw everything else
    DrawDecorations1DPost(image);

	// save the image
    char seedStr[256];
    sprintf(seedStr, "%f", seed);
    char fileName[256];
    sprintf(fileName, "1DGoldenRatio_%s.bmp", &seedStr[2]);
	SaveImage(fileName, image);
}

//======================================================================================
void TestSobol1D ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE1D_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);

	// setup the canvas
	DrawDecorationsPre(image);

	// draw the sample points
	size_t sampleInt = 0;
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
		size_t ruler = Ruler(i + 1);
		size_t direction = size_t(size_t(1) << size_t(31 - ruler));
		sampleInt = sampleInt ^ direction;

		float sample = float(sampleInt) / std::pow(2.0f, 32.0f);
		size_t pos = size_t(sample * float(IMAGE1D_WIDTH)) + IMAGE_PAD;
		ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i));
	}

	// draw everything else
	DrawDecorations1DPost(image);

	// save the image
	SaveImage("1DSobol.bmp", image);
}

//======================================================================================
void TestHammersley1D (size_t truncateBits)
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE1D_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);

	// setup the canvas
	DrawDecorationsPre(image);

	// draw the sample points
	size_t sampleInt = 0;
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
		size_t n = i >> truncateBits;
		float base = 1.0f / 2.0f;
		float sample = 0.0f;
		while (n)
		{
			if (n & 1)
				sample += base;
			n /= 2;
			base /= 2.0f;
		}
		size_t pos = size_t(sample * float(IMAGE1D_WIDTH)) + IMAGE_PAD;
		ImageBox(image, pos, pos + 1, IMAGE1D_CENTERY - DATA_HEIGHT / 2, IMAGE1D_CENTERY + DATA_HEIGHT / 2, DataPointColor(i));
	}

	// draw everything else
	DrawDecorations1DPost(image);

	// save the image
    char fileName[256];
    sprintf(fileName, "1DHammersley_%zu.bmp", truncateBits);
	SaveImage(fileName, image);
}

//======================================================================================
void TestUniform2D (bool jitter)
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE2D_WIDTH + IMAGE_PAD * 2, IMAGE2D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorationsPre(image);

	// draw the sample points
	const size_t c_oneSide = size_t(std::sqrt(NUM_SAMPLES));
	const float c_halfJitter = 1.0f / float((c_oneSide + 1) * 2);
	for (size_t iy = 0; iy < c_oneSide; ++iy)
	{
		for (size_t ix = 0; ix < c_oneSide; ++ix)
		{
			float samplex = float(ix + 1) / (float(c_oneSide) + 1.0f);

			if (jitter)
				samplex += RandomFloat(-c_halfJitter, c_halfJitter);

			size_t posx = size_t(samplex * float(IMAGE2D_WIDTH)) + IMAGE_PAD;

            float sampley = float(iy + 1) / (float(c_oneSide) + 1.0f);

            if (jitter)
                sampley += RandomFloat(-c_halfJitter, c_halfJitter);

            size_t posy = size_t(sampley * float(IMAGE2D_HEIGHT)) + IMAGE_PAD;

			ImageBox(image, posx - 1, posx + 1, posy - 1, posy + 1, DataPointColor(iy*c_oneSide+ix));
		}
	}

    // draw everything else
    DrawDecorations2DPost(image);

	// save the image
    if (jitter)
	    SaveImage("2DUniformJitter.bmp", image);
    else
        SaveImage("2DUniform.bmp", image);
}

//======================================================================================
void TestUniformRandom2D ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE2D_WIDTH + IMAGE_PAD * 2, IMAGE2D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorationsPre(image);

	// draw the sample points
	const size_t c_oneSide = size_t(std::sqrt(NUM_SAMPLES));
	const float c_halfJitter = 1.0f / float((c_oneSide + 1) * 2);
	for (size_t iy = 0; iy < c_oneSide; ++iy)
	{
		for (size_t ix = 0; ix < c_oneSide; ++ix)
		{
            float samplex = RandomFloat(0.0f, 1.0f);
            size_t posx = size_t(samplex * float(IMAGE2D_WIDTH)) + IMAGE_PAD;

            float sampley = RandomFloat(0.0f, 1.0f);
            size_t posy = size_t(sampley * float(IMAGE2D_HEIGHT)) + IMAGE_PAD;

			ImageBox(image, posx - 1, posx + 1, posy - 1, posy + 1, DataPointColor(iy*c_oneSide+ix));
		}
	}

    // draw everything else
    DrawDecorations2DPost(image);

	// save the image
    SaveImage("2DUniformRandom.bmp", image);
}

//======================================================================================
void TestHalton (size_t basex, size_t basey)
{
	// create and clear the image
	SImageData image;
    ImageInit(image, IMAGE2D_WIDTH + IMAGE_PAD * 2, IMAGE2D_HEIGHT + IMAGE_PAD * 2);
	
    // setup the canvas
    DrawDecorationsPre(image);

	// draw the sample points
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
        // x axis
        float samplex = 0.0f;
        {
            float denominator = float(basex);
            size_t n = i;
            while (n > 0)
            {
                size_t multiplier = n % basex;
                samplex += float(multiplier) / denominator;
                n = n / basex;
                denominator *= basex;
            }
        }

        // y axis
        float sampley = 0.0f;
        {
            float denominator = float(basey);
            size_t n = i;
            while (n > 0)
            {
                size_t multiplier = n % basey;
                sampley += float(multiplier) / denominator;
                n = n / basey;
                denominator *= basey;
            }
        }

		size_t posx = size_t(samplex * float(IMAGE2D_WIDTH)) + IMAGE_PAD;
        size_t posy = size_t(sampley * float(IMAGE2D_HEIGHT)) + IMAGE_PAD;

        ImageBox(image, posx - 1, posx + 1, posy - 1, posy + 1, DataPointColor(i));
	}

    // draw everything else
    DrawDecorations2DPost(image);

	// save the image
    char fileName[256];
	sprintf(fileName, "2DHalton_%zu_%zu.bmp", basex, basey);
	SaveImage(fileName, image);
}

//======================================================================================
void TestHammersley2D (size_t truncateBits)
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE2D_WIDTH + IMAGE_PAD * 2, IMAGE2D_HEIGHT + IMAGE_PAD * 2);

	// setup the canvas
	DrawDecorationsPre(image);

    // figure out how many bits we are working in.
    size_t value = 1;
    size_t numBits = 0;
    while (value < NUM_SAMPLES)
    {
        value *= 2;
        ++numBits;
    }

    // TODO: verify 2d is correct vs http://mathworld.wolfram.com/HammersleyPointSet.html
    // * truncating bits is not correct!
    // * 0 truncation seems fine, but the others mismatch somehow

    // TODO: make x axis use mask like y axis does.
    // TODO: make 1d also use mask! cleaner looking imo.

	// draw the sample points
	size_t sampleInt = 0;
	for (size_t i = 0; i < NUM_SAMPLES; ++i)
	{
        // x axis
        float samplex = 0.0f;
        {
            size_t n = i >> truncateBits;
            float base = 1.0f / 2.0f;
            while (n)
            {
                if (n & 1)
                    samplex += base;
                n /= 2;
                base /= 2.0f;
            }
        }

        // y axis
        float sampley = 0.0f;
        {
            size_t maskTruncation = size_t(1) << (truncateBits);
            size_t n = i;
            size_t mask = size_t(1) << (numBits - 1);
            float base = 1.0f / 2.0f;
            while (mask >= maskTruncation)
            {
                if (n & mask)
                    sampley += base;
                mask /= 2;
                base /= 2.0f;
            }
        }

        size_t posx = size_t(samplex * float(IMAGE2D_WIDTH)) + IMAGE_PAD;
        size_t posy = size_t(sampley * float(IMAGE2D_HEIGHT)) + IMAGE_PAD;

        ImageBox(image, posx - 1, posx + 1, posy - 1, posy + 1, DataPointColor(i));
	}

	// draw everything else
	DrawDecorations2DPost(image);

	// save the image
    char fileName[256];
    sprintf(fileName, "2DHammersley_%zu.bmp", truncateBits);
	SaveImage(fileName, image);
}

//======================================================================================
int main (int argc, char **argv)
{
    // 1D tests
    {
        TestUniform1D(false);
        TestUniform1D(true);

        TestUniformRandom1D();

        TestVanDerCorput(2);
        TestVanDerCorput(3);
        TestVanDerCorput(4);
		TestVanDerCorput(5);

        TestGoldenRatio(0.0f);
        TestGoldenRatio(0.385180f);
        TestGoldenRatio(0.775719f);
        TestGoldenRatio(0.287194f);

		TestSobol1D();

		TestHammersley1D(0);
        TestHammersley1D(1);
        TestHammersley1D(2);
    }

	// 2D tests
	{
		TestUniform2D(false);
		TestUniform2D(true);

        TestUniformRandom2D();

        TestHalton(2, 3);
        TestHalton(5, 7);
        TestHalton(13, 9);

        // TODO: Sobol2d

        TestHammersley2D(0);
        TestHammersley2D(1);
        TestHammersley2D(2);

        // TODO: temp!
        TestHammersley2D(3);
        TestHammersley2D(4);
        TestHammersley2D(5);
	}


}

/*

TODO:

* do fft for both 1d and 2d? does it make sense for sample points?
 * if not, how do we analyze quality?
 * maybe we calculate variance? i dunno...

? could it be a generalization of hammersley to not always use base 2? sounds more like van der corput though

* 1d
 ? faure sequence?

* 2d
 * SOBOL 2D is more involved!
 * Hammersly
 ? does fibanoci come in 2d?
 * ??

* van der corput based shuffling.
 * can seed (kind of) by changing base
 * 2d shuffle?

? count the bits to make white noise into gaussian? or save this for another post?

* test on x86 and x64!

BLOG:
? should i show the sequence with fewer points then more and more?
* i tried adding a progressively smaller jitter to van der corput but it seemed to make the sampling worse
* golden ratio, van der corput are nice in that you don't have to know in advance how many samples you want to take.
* 1D sobol (which is a simplified sobol) is actually van der corput sequence re-arranged a little bit.
* truncating 1D hammersley doesn't make sense. it is a way better idea in 2d since it makes more variation.
* halton is 2d van der corput

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

sobol:
http://papa.bretmulvey.com/post/153648811993/sobol-sequences-made-simple
http://web.maths.unsw.edu.au/~fkuo/sobol/

hammersley:
http://mathworld.wolfram.com/HammersleyPointSet.html
 
faure:
http://sas.uwaterloo.ca/~dlmcleis/s906/lds.pdf

*/