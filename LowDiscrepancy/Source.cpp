#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>  // for bitmap headers and performance counter.  Sorry non windows people!
#include <vector>
#include <stdint.h>
#include <random>

typedef uint8_t uint8;

#define IMAGE_WIDTH 400
#define IMAGE1D_HEIGHT 50
#define IMAGE_PAD   30

#define IMAGE_CENTERX ((IMAGE_WIDTH+IMAGE_PAD*2)/2)
#define IMAGE1D_CENTERY ((IMAGE1D_HEIGHT+IMAGE_PAD*2)/2)

#define AXIS_HEIGHT 20
#define DATA_SIZE 10

#define COLOR_FILL SColor(255, 255, 255)
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
void ImageHLine (SImageData& image, size_t x1, size_t x2, size_t y, const SColor& color)
{
	ImageBox(image, x1, x2, y, y + 1, color);
}

//======================================================================================
void ImageVLine (SImageData& image, size_t x, size_t y1, size_t y2, const SColor& color)
{
	ImageBox(image, x, x+1, y1, y2, color);
}

//======================================================================================
void TestUniform1D ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);
	ImageClear(image, COLOR_FILL);

	// draw the sample points
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	for (size_t i = 0; i < 50; ++i)
	{
		float sample = float(i+1) / 51.0f;
		size_t pos = size_t(sample * float(IMAGE_WIDTH)) + IMAGE_PAD;
		ImageVLine(image, pos, IMAGE1D_CENTERY - DATA_SIZE / 2, IMAGE1D_CENTERY + DATA_SIZE / 2, COLOR_DATAPOINT);
	}

	// draw the axes lines
	ImageHLine(image, IMAGE_PAD, IMAGE_WIDTH + IMAGE_PAD, IMAGE1D_CENTERY, COLOR_AXIS);
	ImageVLine(image, IMAGE_PAD, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);
	ImageVLine(image, IMAGE_WIDTH + IMAGE_PAD, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);

	// save the image
	SaveImage("Uniform1D.bmp", image);
}

//======================================================================================
void TestUniformRandom1D ()
{
	// create and clear the image
	SImageData image;
	ImageInit(image, IMAGE_WIDTH + IMAGE_PAD * 2, IMAGE1D_HEIGHT + IMAGE_PAD * 2);
	ImageClear(image, COLOR_FILL);

	// draw the sample points
	static std::random_device rd;
	static std::mt19937 mt(rd());
	static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	for (size_t i = 0; i < 50; ++i)
	{
		float sample = dist(mt);
		size_t pos = size_t(sample * float(IMAGE_WIDTH)) + IMAGE_PAD;
		ImageVLine(image, pos, IMAGE1D_CENTERY - DATA_SIZE / 2, IMAGE1D_CENTERY + DATA_SIZE / 2, COLOR_DATAPOINT);
	}

	// draw the axes lines
	ImageHLine(image, IMAGE_PAD, IMAGE_WIDTH + IMAGE_PAD, IMAGE1D_CENTERY, COLOR_AXIS);
	ImageVLine(image, IMAGE_PAD, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);
	ImageVLine(image, IMAGE_WIDTH + IMAGE_PAD, IMAGE1D_CENTERY - AXIS_HEIGHT / 2, IMAGE1D_CENTERY + AXIS_HEIGHT / 2, COLOR_AXIS);

	// save the image
	SaveImage("UniformRandom1D.bmp", image);
}

//======================================================================================
int main (int argc, char **argv)
{
	TestUniform1D();
	TestUniformRandom1D();
}