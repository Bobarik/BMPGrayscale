#define _CRT_SECURE_NO_WARNINGS
#include "BMPGrayscale.h"

int main(int argc, char** argv) {
	IOFiles inOut = getInOut(argc, argv);
	InfoBMP BMP = getInfoBMP(inOut.input);
	monochromize(inOut, BMP);
	return 0;
}