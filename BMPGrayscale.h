#pragma once
#include <stdio.h>

#define byte unsigned char
#define word unsigned short
#define dword unsigned int
#define WRONG_NAME_OF_FILE -1

struct pixel_16 { //using 2 bytes for pixel
	byte First;
	byte Second;
};

struct pixel_24 { //Using 3 or 4 bytes for pixel
	byte r;
	byte g;
	byte b;
	byte alpha;
};

struct pixel_48 { //Using 3 or 4 words (6 or 8 bytes) for pixel
	word r;
	word g;
	word b;
	word alpha;
};

struct InfoBMP
{
	word endian;
	dword byteSize;
	dword pixelAddress;
	dword byteInfoSize;
	dword width, height;
	dword colorsUsed;
	word bitCount;
	dword Compression;
};

dword LumaGray(dword, dword, dword);

InfoBMP GetInfoBMP(FILE*);
void copyInfo(word, FILE*, FILE*);
void copyPixelStorage(InfoBMP, FILE*, FILE*);

void Pix16(dword, dword, FILE*, FILE*);
void Pix24(dword, dword, FILE*, FILE*, byte);
void Pix48(dword, dword, FILE*, FILE*, byte);
void PixWrite24(FILE*, pixel_24, byte);
pixel_24 Px24(FILE*, byte);