/*===================================================

Written by Alan Wolfe 5/2012

http://demofox.org

some useful links about the wave file format:
http://www.piclist.com/techref/io/serial/midi/wave.html
https://ccrma.stanford.edu/courses/422/projects/WaveFormat/

Note: This source code assumes that you are on a little endian machine.

===================================================*/

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

//define our types.  If your environment varies, you can change these types to be what they should be
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

//some macros
#define CLAMP(value,min,max) {if(value < min) { value = min; } else if(value > max) { value = max; }}

//this struct is the minimal required header data for a wav file
struct SMinimalWaveFileHeader
{
	//the main chunk
	unsigned char m_szChunkID[4];
	uint32		  m_nChunkSize;
	unsigned char m_szFormat[4];

	//sub chunk 1 "fmt "
	unsigned char m_szSubChunk1ID[4];
	uint32		  m_nSubChunk1Size;
	uint16		  m_nAudioFormat;
	uint16		  m_nNumChannels;
	uint32		  m_nSampleRate;
	uint32		  m_nByteRate;
	uint16		  m_nBlockAlign;
	uint16		  m_nBitsPerSample;

	//sub chunk 2 "data"
	unsigned char m_szSubChunk2ID[4];
	uint32		  m_nSubChunk2Size;

	//then comes the data!
};

//0 to 255
void ConvertFloatToAudioSample(float fFloat, uint8 &nOut)
{
	fFloat = (fFloat + 1.0f) * 127.5f;
	CLAMP(fFloat,0.0f,255.0f);
	nOut = (uint8)fFloat;
}

//–32,768 to 32,767
void ConvertFloatToAudioSample(float fFloat, int16 &nOut)
{
	fFloat *= 32767.0f;
	CLAMP(fFloat,-32768.0f,32767.0f);
	nOut = (int16)fFloat;
}

//–2,147,483,648 to 2,147,483,647
void ConvertFloatToAudioSample(float fFloat, int32 &nOut)
{
	double dDouble = (double)fFloat;
	dDouble *= 2147483647.0;
	CLAMP(dDouble,-2147483648.0,2147483647.0);
	nOut = (int32)dDouble;
}

//calculate the frequency of the specified note.
//fractional notes allowed!
float CalcFrequency(float fOctave,float fNote)
/*
	Calculate the frequency of any note!
	frequency = 440×(2^(n/12))

	N=0 is A4
	N=1 is A#4
	etc...

	notes go like so...
	0  = A
	1  = A#
	2  = B
	3  = C
	4  = C#
	5  = D
	6  = D#
	7  = E
	8  = F
	9  = F#
	10 = G
	11 = G#
*/
{
	return (float)(440*pow(2.0,((double)((fOctave-4)*12+fNote))/12.0));
}


//this writes a wave file
//specify sample bit count as the template parameter
//can be uint8, int16 or int32
template <typename T>
bool WriteWaveFile(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate)
{
	//open the file if we can
	FILE *File = fopen(szFileName,"w+b");
	if(!File)
	{
		return false;
	}

	//calculate bits per sample and the data size
	int32 nBitsPerSample = sizeof(T) * 8;
	int nDataSize = nNumSamples * sizeof(T);

	SMinimalWaveFileHeader waveHeader;

	//fill out the main chunk
	memcpy(waveHeader.m_szChunkID,"RIFF",4);
	waveHeader.m_nChunkSize = nDataSize + 36;
	memcpy(waveHeader.m_szFormat,"WAVE",4);

	//fill out sub chunk 1 "fmt "
	memcpy(waveHeader.m_szSubChunk1ID,"fmt ",4);
	waveHeader.m_nSubChunk1Size = 16;
	waveHeader.m_nAudioFormat = 1;
	waveHeader.m_nNumChannels = nNumChannels;
	waveHeader.m_nSampleRate = nSampleRate;
	waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPerSample / 8;
	waveHeader.m_nBlockAlign = nNumChannels * nBitsPerSample / 8;
	waveHeader.m_nBitsPerSample = nBitsPerSample;

	//fill out sub chunk 2 "data"
	memcpy(waveHeader.m_szSubChunk2ID,"data",4);
	waveHeader.m_nSubChunk2Size = nDataSize;

	//write the header
	fwrite(&waveHeader,sizeof(SMinimalWaveFileHeader),1,File);

	//write the wave data itself, converting it from float to the type specified
	T *pData = new T[nNumSamples];
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
		ConvertFloatToAudioSample(pRawData[nIndex],pData[nIndex]);
	fwrite(pData,nDataSize,1,File);
	delete[] pData;

	//close the file and return success
	fclose(File);
	return true;
}

//pass in the current phase, frequency, and sample rate, and these tone generation functions
//will advance the phase and return the sample for that phase

float AdvanceOscilator_Sine(float &fPhase, float fFrequency, float fSampleRate)
{
	fPhase += 2 * (float)M_PI * fFrequency/fSampleRate;

	while(fPhase >= 2 * (float)M_PI)
		fPhase -= 2 * (float)M_PI;

	while(fPhase < 0)
		fPhase += 2 * (float)M_PI;

	return sin(fPhase);
}

float AdvanceOscilator_Saw(float &fPhase, float fFrequency, float fSampleRate)
{
	fPhase += fFrequency/fSampleRate;

	while(fPhase > 1.0f)
		fPhase -= 1.0f;

	while(fPhase < 0.0f)
		fPhase += 1.0f;

	return (fPhase * 2.0f) - 1.0f;
}

float AdvanceOscilator_Square(float &fPhase, float fFrequency, float fSampleRate)
{
	fPhase += fFrequency/fSampleRate;

	while(fPhase > 1.0f)
		fPhase -= 1.0f;

	while(fPhase < 0.0f)
		fPhase += 1.0f;

	if(fPhase <= 0.5f)
		return -1.0f;
	else
		return 1.0f;
}

float AdvanceOscilator_Triangle(float &fPhase, float fFrequency, float fSampleRate)
{
	fPhase += fFrequency/fSampleRate;

	while(fPhase > 1.0f)
		fPhase -= 1.0f;

	while(fPhase < 0.0f)
		fPhase += 1.0f;

	float fRet;
	if(fPhase <= 0.5f)
		fRet=fPhase*2;
	else
		fRet=(1.0f - fPhase)*2;

	return (fRet * 2.0f) - 1.0f;
}

float AdvanceOscilator_Noise(float &fPhase, float fFrequency, float fSampleRate, float fLastValue)
{
	unsigned int nLastSeed = (unsigned int)fPhase;
	fPhase += fFrequency/fSampleRate;
	unsigned int nSeed = (unsigned int)fPhase;

	while(fPhase > 2.0f)
		fPhase -= 1.0f;

	if(nSeed != nLastSeed)
	{
		float fValue = ((float)rand()) / ((float)RAND_MAX);
		fValue = (fValue * 2.0f) - 1.0f;

		//uncomment the below to make it slightly more intense
		/*
		if(fValue < 0)
			fValue = -1.0f;
		else
			fValue = 1.0f;
		*/

		return fValue;
	}
	else
	{
		return fLastValue;
	}
}

//the entry point of our application
int main(int argc, char **argv)
{
	//our parameters that all the wave forms use
	int nSampleRate = 44100;
	int nNumSeconds = 4;
	int nNumChannels = 1;
	float fFrequency = CalcFrequency(3,3); // middle C

	//make our buffer to hold the samples
	int nNumSamples = nSampleRate * nNumChannels * nNumSeconds;
	float *pData = new float[nNumSamples];

	//the phase of our oscilator, we don't really need to reset it between wave files
	//it just needs to stay continuous within a wave file
	float fPhase = 0;

	//make a naive sine wave
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		pData[nIndex] = sin((float)nIndex * 2 * (float)M_PI * fFrequency / (float)nSampleRate);
	}

	WriteWaveFile<int16>("sinenaive.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a discontinuitous (popping) sine wave
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		if(nIndex < nNumSamples / 2)
		{
			float fCurrentFrequency = CalcFrequency(3,3);
			pData[nIndex] = sin((float)nIndex * 2 * (float)M_PI * fCurrentFrequency / (float)nSampleRate);
		}
		else
		{
			float fCurrentFrequency = CalcFrequency(3,4);
			pData[nIndex] = sin((float)nIndex * 2 * (float)M_PI * fCurrentFrequency / (float)nSampleRate);
		}
	}

	WriteWaveFile<int16>("sinediscon.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a continuous sine wave that changes frequencies
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		if(nIndex < nNumSamples / 2)
		{
			float fCurrentFrequency = CalcFrequency(3,3);
			fPhase += 2 * (float)M_PI * fCurrentFrequency/(float)nSampleRate;

			while(fPhase >= 2 * (float)M_PI)
				fPhase -= 2 * (float)M_PI;

			while(fPhase < 0)
				fPhase += 2 * (float)M_PI;

			pData[nIndex] = sin(fPhase);
		}
		else
		{
			float fCurrentFrequency = CalcFrequency(3,4);
			fPhase += 2 * (float)M_PI * fCurrentFrequency/(float)nSampleRate;

			while(fPhase >= 2 * (float)M_PI)
				fPhase -= 2 * (float)M_PI;

			while(fPhase < 0)
				fPhase += 2 * (float)M_PI;

			pData[nIndex] = sin(fPhase);
		}
	}

	WriteWaveFile<int16>("sinecon.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a sine wave
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		pData[nIndex] = AdvanceOscilator_Sine(fPhase,fFrequency,(float)nSampleRate);
	}

	WriteWaveFile<int16>("sine.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a quieter sine wave
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		pData[nIndex] = AdvanceOscilator_Sine(fPhase,fFrequency,(float)nSampleRate) * 0.4f;
	}

	WriteWaveFile<int16>("sinequiet.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a clipping sine wave
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		pData[nIndex] = AdvanceOscilator_Sine(fPhase,fFrequency,(float)nSampleRate) * 1.4f;
	}

	WriteWaveFile<int16>("sineclip.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a saw wave
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		pData[nIndex] = AdvanceOscilator_Saw(fPhase,fFrequency,(float)nSampleRate);
	}

	WriteWaveFile<int16>("saw.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a square wave
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		pData[nIndex] = AdvanceOscilator_Square(fPhase,fFrequency,(float)nSampleRate);
	}

	WriteWaveFile<int16>("square.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a triangle wave
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		pData[nIndex] = AdvanceOscilator_Triangle(fPhase,fFrequency,(float)nSampleRate);
	}

	WriteWaveFile<int16>("triangle.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make some noise or... make... some... NOISE!!!
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		pData[nIndex] = AdvanceOscilator_Noise(fPhase,fFrequency,(float)nSampleRate, nIndex > 0 ? pData[nIndex-1] : 0.0f);
	}

	WriteWaveFile<int16>("noise.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//make a dumb little song
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
	{
		//calculate which quarter note we are on
		int nQuarterNote = nIndex * 4 / nSampleRate;
		float fQuarterNotePercent = (float)((nIndex * 4) % nSampleRate) / (float)nSampleRate;

		//intentionally add a "pop" noise mid way through the 3rd quarter note
		if(nIndex == nSampleRate * 3 / 4 + nSampleRate / 8)
		{
			pData[nIndex] = -1.0f;
			continue;
		}

		//do different logic based on which quarter note we are on
		switch(nQuarterNote)
		{
			case 0:
			{
				pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,0),(float)nSampleRate);
				break;
			}
			case 1:
			{
				pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,2),(float)nSampleRate);
				break;
			}
			case 2:
			case 3:
			{
				pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,5),(float)nSampleRate);
				break;
			}
			case 4:
			{
				pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,5 - fQuarterNotePercent * 2.0f),(float)nSampleRate);
				break;
			}
			case 5:
			{
				pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,3 + fQuarterNotePercent * 2.0f),(float)nSampleRate);
				break;
			}
			case 6:
			{
				pData[nIndex] = AdvanceOscilator_Sine(fPhase,CalcFrequency(4,5 - fQuarterNotePercent * 2.0f),(float)nSampleRate) * (1.0f - fQuarterNotePercent);
				break;
			}

			case 8:
			{
				pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,0),(float)nSampleRate);
				break;
			}
			case 9:
			{
				pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,2),(float)nSampleRate);
				break;
			}
			case 10:
			case 11:
			{
				pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,5),(float)nSampleRate);
				break;
			}
			case 12:
			{
				pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,5 - fQuarterNotePercent * 2.0f),(float)nSampleRate);
				break;
			}
			case 13:
			{
				pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,3 + fQuarterNotePercent * 2.0f),(float)nSampleRate);
				break;
			}
			case 14:
			{
				pData[nIndex] = AdvanceOscilator_Saw(fPhase,CalcFrequency(4,5 - fQuarterNotePercent * 2.0f),(float)nSampleRate) * (1.0f - fQuarterNotePercent);
				break;
			}

			default:
			{
				pData[nIndex] = 0;
				break;
			}
		}
	}

	WriteWaveFile<int16>("song.wav",pData,nNumSamples,nNumChannels,nSampleRate);

	//free our data buffer
	delete[] pData;
}