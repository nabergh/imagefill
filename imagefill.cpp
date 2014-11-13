#include "CImg.h"
using namespace cimg_library;


CImg<unsigned char> source;
CImg<unsigned char> matte;
CImg<unsigned char> result;


int main(int argc, char *argv[]) {
	if (argc > 3) {
		source = CImg<unsigned char>(argv[1]);
		source.display();
	} else {
		printf("You must parameters like so: /imagequilt [in file] [matte file] [out file]\n");
		return 0;
	}

	return 0;
}