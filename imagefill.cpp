#include "CImg.h"
#include "math.h"
#include <vector>
using namespace cimg_library;


CImg<unsigned char> source;
CImg<unsigned char> matte;
CImg<unsigned char> result;
CImg<float> confidence_values;
CImg<bool> omega;
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
	int i, j;
	float temp_conf = 0;
	int area = (min(p.x_loc + 4, source.width() - 1) - max(p.x_loc - 4, 0)) - (min(p.y_loc + 4, source.height() - 1) - max(p.y_loc - 4, 0));
	for (i = max(p.x_loc - 4, 0); i <= min(p.x_loc + 4, source.width() - 1); i++) {
		for (j = max(p.y_loc - 4, 0); j <= min(p.y_loc + 4, source.height() - 1); j++) {
			if (!omega(i,j)) {
				temp_conf = temp_conf + confidence_values(i, j);
			}
		}
	}
	p.conf = temp_conf/area;
	confidence_values(p.x_loc, p.y_loc) = p.conf;
}

void init(int width, int height, int square_x, int square_y, int square_size) {
	confidence_values = CImg<float>(source.width(), source.height(), 1, 1, 1);
	omega = CImg<bool>(source.width(), source.height(), 1, 1, 0);
	int i, j;
	for (i = square_x; i < square_x+square_size; i++) {
		for (j = square_y; j < square_y+square_size; j++) {
			confidence_values(i, j) = 0;
			omega(i, j) = 1;
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
		init(source.width(), source.height(), squareleft, squaretop, squaresize);
		confidence_values.display();
		source.display();
	} else {
		printf("You must parameters like so: /imagequilt [in file] [square size]\n");
		return 0;
	}

	return 0;
}