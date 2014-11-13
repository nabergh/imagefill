#include "CImg.h"
#include "math.h"
#include "vector.h"
using namespace cimg_library;


CImg<unsigned char> source;
CImg<unsigned char> matte;
CImg<unsigned char> result;
int squareleft;
int squareright;
int squaretop;
int squarebottom;
int squaresize;
std::vector<pixel_info> fillfront;

struct {
	int x_loc;
	int y_loc;
	float conf;
	float data;
	float priority;
} pixel_info;

float confidence() {
	return 0;
}


int main(int argc, char *argv[]) {
	if (argc > 3) {
		source = CImg<unsigned char>(argv[1]);
		squaresize = atoi(argv[2]);
		srand (time(NULL));
		squareleft = rand() % (source.width() - squaresize);
		squaretop = rand() % (source.height() - squaresize);
		squareright = squareleft + squaresize;
		squarebottom = squaretop + squaresize;

		source.display();
	} else {
		printf("You must parameters like so: /imagequilt [in file] [square size]\n");
		return 0;
	}

	return 0;
}