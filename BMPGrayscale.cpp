#include "BMPGrayscale.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>

// Using "Luma" formula to count the shade of gray for each pixel.
dword LumaGray(dword r, dword g, dword b) {
	return (dword)(b * 0.0722 + r * 0.2125 + g * 0.7151);
}

//Reading Info to create InfoBMP variable
InfoBMP GetInfoBMP(FILE* input) {
	InfoBMP BMP;

	fread(&BMP.endian, sizeof(word), 1, input); // if it's BMP
	if (BMP.endian != 19778 && BMP.endian != 16973) {
		printf("Wrong format of file.");
		_getch();
		exit(WRONG_NAME_OF_FILE);
	}

	fread(&BMP.byteSize, sizeof(int), 1, input); // Size of file.
	fseek(input, 4, SEEK_CUR);
	fread(&BMP.pixelAddress, sizeof(dword), 1, input); // Beginning of pixel storage.

	fread(&BMP.byteInfoSize, sizeof(dword), 1, input); // Version of BitInfo by size of it.

	if (BMP.byteInfoSize == 12) { // Different types of width and height depending on version.
		fread(&BMP.width, sizeof(word), 1, input);
		fread(&BMP.height, sizeof(word), 1, input);

		fseek(input, 28, SEEK_SET);
		fread(&BMP.bitCount, sizeof(word), 1, input); // Amount of bits per pixel.
	}
	else {
		fread(&BMP.width, sizeof(dword), 1, input);
		fread(&BMP.height, sizeof(dword), 1, input);


		fseek(input, 28, SEEK_SET);
		fread(&BMP.bitCount, sizeof(word), 1, input); // Amount of bits per pixel.

													  //If it isn't CORE version of BMP, read this:
		fread(&BMP.Compression, sizeof(dword), 1, input); // Type of Compression.
		fseek(input, 46, SEEK_SET);
		fread(&BMP.colorsUsed, sizeof(dword), 1, input); // If table of colors is used and how many colors it have.
	}
	return BMP;
}

// Copying info from original Header + Info
void copyInfo(word infoSize, FILE* input, FILE* output) {
	byte add;
	fseek(input, 0, SEEK_SET);
	for (int i = 0; i < infoSize + 14; i++) {
		fread(&add, sizeof(byte), 1, input);
		fwrite(&add, sizeof(byte), 1, output);
	}
}

void copyPixelStorage(InfoBMP BMP, FILE* input, FILE* output) {
	byte add;
	fseek(input, BMP.pixelAddress, SEEK_SET);
	for (dword i = BMP.pixelAddress; i < BMP.byteSize; i++) {
		fread(&add, sizeof(byte), 1, input);
		fwrite(&add, sizeof(byte), 1, output);
	}
}

void PixWrite24(FILE* output, pixel_24 pix, byte is32) {
	fwrite(&pix.b, sizeof(byte), 1, output);
	fwrite(&pix.g, sizeof(byte), 1, output);
	fwrite(&pix.r, sizeof(byte), 1, output);
	if (is32) {
		fwrite(&pix.alpha, sizeof(byte), 1, output);
	}
}

pixel_24 Px24(FILE* input, byte is32) {
	pixel_24 pix;
	byte gray;
	fread(&pix.b, sizeof(byte), 1, input);
	fread(&pix.g, sizeof(byte), 1, input);
	fread(&pix.r, sizeof(byte), 1, input);
	if (is32) {
		fread(&pix.alpha, sizeof(byte), 1, input); 
	}
	gray = (byte)LumaGray(pix.r, pix.g, pix.b); // Using "Luma" formula to determine gray for each pixel.
	pix.r = gray;
	pix.b = gray;
	pix.g = gray;
	return pix;
}

void PixWrite48(FILE* output, pixel_48 pix, byte is64) {
	fwrite(&pix.b, sizeof(word), 1, output);
	fwrite(&pix.g, sizeof(word), 1, output);
	fwrite(&pix.r, sizeof(word), 1, output);
	if (is64) {
		fwrite(&pix.alpha, sizeof(word), 1, output);
	}
}

pixel_48 Px48(FILE* input, byte is64) {
	pixel_48 pix;
	byte gray;
	fread(&pix.b, sizeof(word), 1, input);
	fread(&pix.g, sizeof(word), 1, input);
	fread(&pix.r, sizeof(word), 1, input);
	if (is64) {
		fread(&pix.alpha, sizeof(word), 1, input);
	}
	gray = (word)LumaGray(pix.r, pix.g, pix.b); // Using "Luma" formula to determine gray for each pixel.
	pix.r = gray;
	pix.b = gray;
	pix.g = gray;
	return pix;
}

//16 Bit per pixel
void Pix16(dword height, dword width, FILE* input, FILE* output) {
	pixel_16 pix;
	pixel_24 pix24; //using pix24 for better understanding of 16-pix pixel.
	byte gray;
	byte additional = 0;
	byte additionalBytes = (4 - (width * 3 % 4)) % 4;
	for (dword i = 0; i < height; i++) {
		for (dword j = 0; j < width; j++) {
			fread(&pix.First, sizeof(byte), 1, input);
			fread(&pix.Second, sizeof(byte), 1, input);
			pix24.b = pix.First / 8; // First 5 bits of 1st byte
			pix24.g = pix.First % 8 * 4 + pix.Second / 64; // last 3 bits of 1st byte, 2 bits of 2nd byte
			pix24.r = (pix.Second / 2) % 32; // 3 to 7th bits of 2nd byte
			pix24.alpha = pix.Second % 2; //last bit of 2nd byte
			gray = LumaGray(pix24.r, pix24.g, pix24.b); // Using "Luma" formula to determine gray for each pixel.
			pix.First = pix24.b % 32 * 8 + pix24.g % 32 / 4;
			pix.Second = pix24.g % 4 * 64 + pix24.r % 32 * 2 + pix24.alpha % 2;
		}
		fwrite(&additional, sizeof(byte), additionalBytes, output);
		fseek(input, additionalBytes, SEEK_CUR);
	}
}

//24 or 32 Bit per pixel 
void Pix24(dword height, dword width, FILE* input, FILE* output, byte is32) { 
	pixel_24 pix;
	byte additional = 0;
	byte additionalBytes = (4 - (width * (3 + is32) % 4)) % 4;
	for (dword i = 0; i < height; i++) {
		for (dword j = 0; j < width; j++) {
			pix = Px24(input, is32);
			PixWrite24(output, pix, is32);
		}
		fwrite(&additional, sizeof(byte), additionalBytes, output);
		fseek(input, additionalBytes, SEEK_CUR);
	}
}

//48 Bit per pixel: Changig all 'byte' variables in pixel to 'word' (2-byte) variables
void Pix48(dword height, dword width, FILE* input, FILE* output, byte is64) {
	pixel_48 pix;
	word gray;
	byte additional = 0;
	word additionalBytes = (4 - (width * 6 % 4)) % 4;
	for (dword i = 0; i < height; i++) {
		for (dword j = 0; j < width; j++) {
			pix = Px48(input, is64);
			PixWrite48(output, pix, is64);
		}
		additional = 0;
		fwrite(&additional, sizeof(byte), additionalBytes, output);
		fseek(input, additionalBytes, SEEK_CUR);
	}
}

//64 Bit per pixel: Adding alpha-channel. No need for additional bytes in the end (number of bytes is always multiple of 4)
void Pix64(dword height, dword width, FILE* input, FILE* output) {
	pixel_48 pix;
	word gray;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			fread(&pix.b, sizeof(word), 1, input);
			fread(&pix.g, sizeof(word), 1, input);
			fread(&pix.r, sizeof(word), 1, input);
			fread(&pix.alpha, sizeof(word), 1, input);
			gray = LumaGray(pix.r, pix.g, pix.b); // Using "Luma" formula to determine gray for each pixel.
			pix.r = gray;
			pix.b = gray;
			pix.g = gray;
			fwrite(&pix.b, sizeof(word), 1, output);
			fwrite(&pix.g, sizeof(word), 1, output);
			fwrite(&pix.r, sizeof(word), 1, output);
			fwrite(&pix.alpha, sizeof(word), 1, output);
		}
	}
}