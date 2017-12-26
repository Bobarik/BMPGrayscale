#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <conio.h>
#include "BMPGrayscale.h"

BYTE findArg(int argc, char **argv, char* arg) {
	DWORD argLen = strlen(arg);
	int i = 1;
	for (; i < argc && strncmp(argv[i], arg, argLen); i++);
	return (i % argc); // return 0 when i == argc.
}

int main(int argc, char** argv) {
	if (argc >= 2) {
		FILE* input;
		FILE* output;
		char* cinput;
		char* coutput;
		BYTE inIn = findArg(argc, argv, "--input");
		BYTE outIn = findArg(argc, argv, "--output");

		if (inIn != 0 && outIn != 0) {
			cinput = (char*)malloc(sizeof(char) * (strlen(argv[inIn]) - 7));
			memcpy(cinput, argv[inIn] + 8, strlen(argv[inIn]) - 7);
			fopen_s(&input, cinput, "rb");

			coutput = (char*)malloc(sizeof(char) * (strlen(argv[outIn]) - 8));
			memcpy(coutput, argv[outIn] + 9, strlen(argv[outIn]) - 8);
			fopen_s(&output, coutput, "wb");
		} else {
			if (inIn == 0) {
				printf("Wrong arguments: no argument with \"--input\"");
				_getch();
				exit(WRONG_INPUT);
			} else {
				cinput = (char*)malloc(sizeof(char) * (strlen(argv[inIn]) - 7));
				memcpy(cinput, argv[inIn] + 8, strlen(argv[inIn]) - 7);
				fopen_s(&input, cinput, "rb");

				coutput = (char*)malloc(sizeof(char) * (strlen(cinput) + 5));
				memcpy(coutput, "Gray", 4);
				memcpy(coutput + 4, cinput, strlen(cinput) + 1);
				fopen_s(&output, coutput, "wb");

				printf("No arguments with \"--output\". Your output file will be: ");
				puts(coutput);
				printf("Press any key to continue.\n");
				_getch();
			}
		}
		
		if (input == NULL) {
			printf("Wrong name of input file.");
			_getch();
			exit(WRONG_NAME_OF_FILE);
		}

		InfoBMP BMP = GetInfoBMP(input);

		rewind(input);

		if ((BMP.colorsUsed == 0) && (BMP.bitCount > 8) || (BMP.byteInfoSize == 12)) { // Changing two-dimensinoal array for pixel.
			copyInfo(BMP.byteInfoSize, input, output);
			fseek(input, BMP.pixelAddress, SEEK_SET);
			switch (BMP.bitCount) { //grabbing each pixel and changing it.
			case 16:
				Pix16(BMP.height, BMP.width, input, output);
			case 24:
				Pix24(BMP.height, BMP.width, input, output, 0);
				break;
			case 32:
				Pix24(BMP.height, BMP.width, input, output, 1);
				break;
			case 48:
				Pix48(BMP.height, BMP.width, input, output, 0);
				break;
			case 64:
				Pix48(BMP.height, BMP.width, input, output, 1);
				break;
			}
		} else {
			if (BMP.bitCount <= 8 || BMP.colorsUsed > 0) { //Changing table of colors to grayscale.
				pixel_24 color;
				WORD ColCount = (BMP.colorsUsed != 0 ? BMP.colorsUsed : (WORD)pow(2, BMP.bitCount));
				copyInfo(BMP.byteInfoSize, input, output);
				for (int i = 0; i < ColCount; i++) {
					color = Px24(input, (BMP.byteInfoSize >= 40 ? 1 : 0));
					PixWrite24(output, color, (BMP.byteInfoSize >= 40 ? 1 : 0));
				}
				copyPixelStorage(BMP, input, output);
			}
		}

		_fcloseall();

		printf("Monochromization complete.");
		_getch();

		return 0;
	} else {
		printf("Wrong number of arguments.");
		_getch();
		exit(WRONG_NAME_OF_FILE);
	}
}