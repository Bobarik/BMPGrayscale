#include "BMPGrayscale.h"
#include <conio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Using "Luma" formula to count the shade of gray for each pixel.
DWORD LumaGray(DWORD r, DWORD g, DWORD b) {
	return (DWORD)(b * 0.0722 + r * 0.2125 + g * 0.7151);
}

BYTE findArg(int argc, char **argv, char* arg) {
	DWORD argLen = strlen(arg);
	int i = 1;
	for (; i < argc && strncmp(argv[i], arg, argLen); i++);
	return (i % argc); // return 0 when i == argc.
}

//Finding input and output files from arguments.
IOFiles getInOut(int argc, char** argv) {
	if (argc >= 2) {
		IOFiles IO;
		char* cinput;
		char* coutput;
		BYTE inIn = findArg(argc, argv, "--input");
		BYTE outIn = findArg(argc, argv, "--output");

		if (inIn != 0 && outIn != 0) {
			cinput = (char*)malloc(sizeof(char) * (strlen(argv[inIn]) - 7));
			memcpy(cinput, argv[inIn] + 8, strlen(argv[inIn]) - 7);
			fopen_s(&IO.input, cinput, "rb");

			coutput = (char*)malloc(sizeof(char) * (strlen(argv[outIn]) - 8));
			memcpy(coutput, argv[outIn] + 9, strlen(argv[outIn]) - 8);
			fopen_s(&IO.output, coutput, "wb");
		} else {
			if (inIn == 0) {
				printf("Wrong arguments: no argument with \"--input\"");
				_getch();
				exit(WRONG_INPUT);
			} else {
				cinput = (char*)malloc(sizeof(char) * (strlen(argv[inIn]) - 7));
				memcpy(cinput, argv[inIn] + 8, strlen(argv[inIn]) - 7);
				fopen_s(&IO.input, cinput, "rb");

				coutput = (char*)malloc(sizeof(char) * (strlen(cinput) + 5));
				memcpy(coutput, "Gray", 4);
				memcpy(coutput + 4, cinput, strlen(cinput) + 1);
				fopen_s(&IO.output, coutput, "wb");

				printf("No arguments with \"--output\". Your output file will be: ");
				puts(coutput);
				printf("Press any key to continue.\n");
				_getch();
			}
		}

		free(cinput);
		free(coutput);

		if (IO.input == NULL) {
			printf("Wrong name of input file.");
			_getch();
			exit(WRONG_NAME_OF_FILE);
		}

		return IO;
	} else {
		printf("Wrong number of arguments.");
		_getch();
		exit(WRONG_NAME_OF_FILE);
	}
}

//Reading Info to create InfoBMP variable
InfoBMP getInfoBMP(FILE* input) {
	InfoBMP BMP;

	fread(&BMP.endian, sizeof(WORD), 1, input); // if it's BMP
	if (BMP.endian != 19778 && BMP.endian != 16973) {
		printf("Wrong format of file.");
		_getch();
		exit(WRONG_NAME_OF_FILE);
	}

	fread(&BMP.byteSize, sizeof(int), 1, input); // Size of file.
	fseek(input, 4, SEEK_CUR);
	fread(&BMP.pixelAddress, sizeof(DWORD), 1, input); // Beginning of pixel storage.

	fread(&BMP.byteInfoSize, sizeof(DWORD), 1, input); // Version of BitInfo by size of it.

	fread(&BMP.width, sizeof(WORD), (BMP.byteInfoSize == 12 ? 1 : 2), input); //Width and height of picture
	fread(&BMP.height, sizeof(WORD), (BMP.byteInfoSize == 12 ? 1 : 2), input);
	
	fseek(input, 28, SEEK_SET);
	fread(&BMP.bitCount, sizeof(WORD), 1, input); // Amount of bits per pixel.

	if (BMP.byteInfoSize != 12) {
		fread(&BMP.Compression, sizeof(DWORD), 1, input); // Type of Compression.
		fseek(input, 46, SEEK_SET);
		fread(&BMP.colorsUsed, sizeof(DWORD), 1, input); // If table of colors is used and how many colors it have.
	}
	return BMP;
}

//Structurization of all methods of monochromization.
void monochromize(IOFiles inOut, InfoBMP BMP) {
	rewind(inOut.input);
	if ((BMP.colorsUsed == 0) && (BMP.bitCount > 8) || (BMP.byteInfoSize == 12)) { // Changing two-dimensinoal array for pixel.
		copyInfo(BMP.byteInfoSize, inOut.input, inOut.output);
		fseek(inOut.input, BMP.pixelAddress, SEEK_SET);
		switch (BMP.bitCount) { //grabbing each pixel and changing it.
		case 16:
			Pix16(BMP.height, BMP.width, inOut.input, inOut.output);
		case 24:
			Pix24(BMP.height, BMP.width, inOut.input, inOut.output, 0);
			break;
		case 32:
			Pix24(BMP.height, BMP.width, inOut.input, inOut.output, 1);
			break;
		case 48:
			Pix48(BMP.height, BMP.width, inOut.input, inOut.output, 0);
			break;
		case 64:
			Pix48(BMP.height, BMP.width, inOut.input, inOut.output, 1);
			break;
		}
	} else {
		if (BMP.bitCount <= 8 || BMP.colorsUsed > 0) { //Changing table of colors to grayscale.
			pixel_24 color;
			WORD ColCount = (BMP.colorsUsed != 0 ? BMP.colorsUsed : (WORD)pow(2, BMP.bitCount));
			copyInfo(BMP.byteInfoSize, inOut.input, inOut.output);
			for (int i = 0; i < ColCount; i++) {
				color = Px24(inOut.input, (BMP.byteInfoSize >= 40 ? 1 : 0));
				PixWrite24(inOut.output, color, (BMP.byteInfoSize >= 40 ? 1 : 0));
			}
			copyPixelStorage(BMP, inOut.input, inOut.output);
		}
	}

	_fcloseall();

	printf("Monochromization complete.");
	_getch();
}

// Copying info from original Header + Info
void copyInfo(WORD infoSize, FILE* input, FILE* output) {
	BYTE* add =(BYTE*) malloc(sizeof(BYTE) * infoSize + 14);
	fseek(input, 0, SEEK_SET);
	fread(add, sizeof(BYTE), infoSize + 14, input);
	fwrite(add, sizeof(BYTE), infoSize + 14, output);
}

// Copying Pixel Storage from original picture. If used, it means that monochromization is completed by the table of colors.
void copyPixelStorage(InfoBMP BMP, FILE* input, FILE* output) {
	BYTE* add = NULL;
	int addSize = BMP.byteSize - BMP.pixelAddress;
	fseek(input, BMP.pixelAddress, SEEK_SET);

	for (; add == NULL; addSize --) {
		add = (BYTE*)malloc(sizeof(BYTE) * (BMP.byteSize - BMP.pixelAddress)); //Trying to allocate memory for whole pixel storage.
	}
	addSize ++; //Compensating for addSize-- on the last iteration.
	for (int i = BMP.byteSize - BMP.pixelAddress; i > 0; i -= addSize) {
		fread(add, (addSize < i ? sizeof(add) : sizeof(BYTE) * i), 1, input);
		fwrite(add, (addSize < i ? sizeof(add) : sizeof(BYTE) * i), 1, output);
	}
	free(add);
}

// Writing 3- or 4-byte pixel into output.   
void PixWrite24(FILE* output, pixel_24 pix, BYTE is32) {
	fwrite(&pix.b, sizeof(BYTE), 1, output);
	fwrite(&pix.g, sizeof(BYTE), 1, output);
	fwrite(&pix.r, sizeof(BYTE), 1, output);
	if (is32) {
		fwrite(&pix.alpha, sizeof(BYTE), 1, output);
	}
}

// Reading and monochromizing 3- or 4- byte pixel.
pixel_24 Px24(FILE* input, BYTE is32) {
	pixel_24 pix;
	BYTE gray;
	fread(&pix.b, sizeof(BYTE), 1, input);
	fread(&pix.g, sizeof(BYTE), 1, input);
	fread(&pix.r, sizeof(BYTE), 1, input);
	if (is32) {
		fread(&pix.alpha, sizeof(BYTE), 1, input); 
	}
	gray = (BYTE)LumaGray(pix.r, pix.g, pix.b); // Using "Luma" formula to determine gray for each pixel.
	pix.r = gray;
	pix.b = gray;
	pix.g = gray;
	return pix;
}

// Writing 6- or 8-byte pixel into output.
void PixWrite48(FILE* output, pixel_48 pix, BYTE is64) {
	fwrite(&pix.b, sizeof(WORD), 1, output);
	fwrite(&pix.g, sizeof(WORD), 1, output);
	fwrite(&pix.r, sizeof(WORD), 1, output);
	if (is64) {
		fwrite(&pix.alpha, sizeof(WORD), 1, output);
	}
}

// Reading and monochromizing 6- or 8- byte pixel.
pixel_48 Px48(FILE* input, BYTE is64) {
	pixel_48 pix;
	BYTE gray;
	fread(&pix.b, sizeof(WORD), 1, input);
	fread(&pix.g, sizeof(WORD), 1, input);
	fread(&pix.r, sizeof(WORD), 1, input);
	if (is64) {
		fread(&pix.alpha, sizeof(WORD), 1, input);
	}
	gray = (WORD)LumaGray(pix.r, pix.g, pix.b); // Using "Luma" formula to determine gray for each pixel.
	pix.r = gray;
	pix.b = gray;
	pix.g = gray;
	return pix;
}

//Monochromizing 2-byte pixels
void Pix16(DWORD height, DWORD width, FILE* input, FILE* output) {
	pixel_16 pix;
	pixel_24 pix24; //using pix24 for better understanding of 16-pix pixel.
	BYTE gray;
	BYTE additional = 0;
	BYTE additionalBytes = (4 - (width * 3 % 4)) % 4;
	for (DWORD i = 0; i < height; i++) {
		for (DWORD j = 0; j < width; j++) {
			fread(&pix.First, sizeof(BYTE), 1, input);
			fread(&pix.Second, sizeof(BYTE), 1, input);
			pix24.b = pix.First / 8; // First 5 bits of 1st BYTE
			pix24.g = pix.First % 8 * 4 + pix.Second / 64; // last 3 bits of 1st BYTE, 2 byte of 2nd byte
			pix24.r = (pix.Second / 2) % 32; // 3 to 7th bits of 2nd byte
			pix24.alpha = pix.Second % 2; //last bit of 2nd byte
			gray = LumaGray(pix24.r, pix24.g, pix24.b); // Using "Luma" formula to determine gray for each pixel.
			pix.First = pix24.b % 32 * 8 + pix24.g % 32 / 4;
			pix.Second = pix24.g % 4 * 64 + pix24.r % 32 * 2 + pix24.alpha % 2;
		}
		fwrite(&additional, sizeof(BYTE), additionalBytes, output);
		fseek(input, additionalBytes, SEEK_CUR);
	}
}

//Monochromizing 3- or 4-byte pixels
void Pix24(DWORD height, DWORD width, FILE* input, FILE* output, BYTE is32) { 
	pixel_24 pix;
	BYTE additional = 0;
	BYTE additionalBytes = (4 - (width * (3 + is32) % 4)) % 4;
	for (DWORD i = 0; i < height; i++) {
		for (DWORD j = 0; j < width; j++) {
			pix = Px24(input, is32);
			PixWrite24(output, pix, is32);
		}
		fwrite(&additional, sizeof(BYTE), additionalBytes, output);
		fseek(input, additionalBytes, SEEK_CUR);
	}
}

//Monochromizing 6- or 8-byte pixels: Changed all 'BYTE' variables in pixel to 'WORD' (2-byte) variables
void Pix48(DWORD height, DWORD width, FILE* input, FILE* output, BYTE is64) {
	pixel_48 pix;
	WORD gray;
	BYTE additional = 0;
	WORD additionalBytes = (4 - (width * 6 % 4)) % 4;
	for (DWORD i = 0; i < height; i++) {
		for (DWORD j = 0; j < width; j++) {
			pix = Px48(input, is64);
			PixWrite48(output, pix, is64);
		}
		fwrite(&additional, sizeof(BYTE), additionalBytes, output);
		fseek(input, additionalBytes, SEEK_CUR);
	}
}