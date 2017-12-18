#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <conio.h>
#include "BMPGrayscale.h"

int main(int argc, char** argv) {
	if (argc >= 2) {
		FILE* input;
		fopen_s(&input, argv[1], "rb");

		if (input == NULL) {
			printf("Wrong name of file.");
			_getch();
			exit(WRONG_NAME_OF_FILE);
		}

		InfoBMP BMP = GetInfoBMP(input);

		FILE* output;
		if (argc == 2) {
			char* coutput = (char*)malloc(sizeof(char) * (strlen(argv[1]) + 5));
			memcpy(coutput, "Gray", 4);
			memcpy(coutput + 4, argv[1], strlen(argv[1]) + 1);
			fopen_s(&output, coutput, "wb");
		}
		else {
			fopen_s(&output, argv[2], "wb");
		}

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
		}
		else {
			if (BMP.bitCount <= 8 || BMP.colorsUsed > 0) { //Changing table of colors to grayscale.
				pixel_24 color;
				word ColCount = (BMP.colorsUsed != 0 ? BMP.colorsUsed : (word)pow(2, BMP.bitCount));
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
	}
	else {
		printf("Wrong number of arguments.");
		_getch();
		exit(WRONG_NAME_OF_FILE);
	}
}