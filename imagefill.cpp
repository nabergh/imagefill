#include "CImg.h"
using namespace cimg_library;


CImg<unsigned char> source;
CImg<unsigned char> matte;
CImg<unsigned char> result;
int squaresize;


int main(int argc, char *argv[]) {
	if (argc > 2) {
		source = CImg<unsigned char>(argv[1]);
		squaresize = atoi(argv[2]);
		source.display();
	} else {
		printf("You must parameters like so: /imagequilt [in file] [square size]\n");
		return 0;
	}

	return 0;
}