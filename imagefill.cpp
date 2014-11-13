#include "CImg.h"
#include "math.h"
#include <vector>
using namespace cimg_library;


CImg<unsigned char> source;
CImg<unsigned char> matte;
CImg<unsigned char> result;
CImg<float> confidence_values;
int squareleft;
int squareright;
int squaretop;
int squarebottom;
int squaresize;

struct pixel_info {
	int x_loc;
	int y_loc;
	float conf;
	float data;
	float priority;
};

std::vector< pixel_info > fillfront;

void confidence(pixel_info & p) {

}

void init_confidence(int width, int height, int square_x, int square_y, int square_size) {
	confidence_values = CImg<float>(source.width(), source.height(), 1, 1, 1);
	int i, j;
	for (i = square_x; i < square_x+square_size; i++) {
		for (j = square_y; j < square_y+square_size; j++) {
			confidence_values(i, j) = 0;
		}
	}

}


int main(int argc, char *argv[]) {
	if (argc == 3) {
		source = CImg<unsigned char>(argv[1]);
		squaresize = atoi(argv[2]);
		srand (time(NULL));
		squareleft = rand() % (source.width() - squaresize);
		squaretop = rand() % (source.height() - squaresize);
		squareright = squareleft + squaresize;
		squarebottom = squaretop + squaresize;
		init_confidence(source.width(), source.height(), squareleft, squaretop, squaresize);
		confidence_values.display();
		source.display();
	} else {
		printf("You must parameters like so: /imagequilt [in file] [square size]\n");
		return 0;
	}

	return 0;
}