#pragma once
#include <stdio.h>

#define BYTE unsigned char
#define WORD unsigned short
#define DWORD unsigned int
#define WRONG_NAME_OF_FILE 3
#define WRONG_INPUT 4

struct pixel_16 { //using 2 bytes for pixel
	BYTE First;
	BYTE Second;
};

struct pixel_24 { //Using 3 or 4 bytes for pixel
	BYTE r;
	BYTE g;
	BYTE b;
	BYTE alpha;
};

struct pixel_48 { //Using 3 or 4 words (6 or 8 bytes) for pixel
	WORD r;
	WORD g;
	WORD b;
	WORD alpha;
};

struct InfoBMP
{
	WORD endian;
	DWORD byteSize;
	DWORD pixelAddress;
	DWORD byteInfoSize;
	DWORD width, height;
	DWORD colorsUsed;
	WORD bitCount;
	DWORD Compression;
};

DWORD LumaGray(DWORD, DWORD, DWORD);

InfoBMP GetInfoBMP(FILE*);
void copyInfo(WORD, FILE*, FILE*);
void copyPixelStorage(InfoBMP, FILE*, FILE*);

void Pix16(DWORD, DWORD, FILE*, FILE*);
void Pix24(DWORD, DWORD, FILE*, FILE*, BYTE);
void Pix48(DWORD, DWORD, FILE*, FILE*, BYTE);
void PixWrite24(FILE*, pixel_24, BYTE);
pixel_24 Px24(FILE*, BYTE);