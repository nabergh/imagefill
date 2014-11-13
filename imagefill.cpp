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

class Vector2d {
public:
	float x;
	float y;
	Vector2d() {
		x = 0;
		y = 0;
	};
	Vector2d(float &x_coord, float &y_coord) {
		x = x_coord;
		y = y_coord;
	};
	void normalize() {
		float length = std::sqrt(y * y + x * x);
		x /= length;
		y /= length;
	};
	float operator*(Vector2d &v1) {
		return x * v1.x + y * v1.y;
	};
};

void confidence(pixel_info &p) {
	int i, j;
	float temp_conf = 0;
	int area = (std::min(p.x_loc + 4, source.width() - 1) - std::max(p.x_loc - 4, 0)) - (std::min(p.y_loc + 4, source.height() - 1) - std::max(p.y_loc - 4, 0));
	for (i = std::max(p.x_loc - 4, 0); i <= std::min(p.x_loc + 4, source.width() - 1); i++) {
		for (j = std::max(p.y_loc - 4, 0); j <= std::min(p.y_loc + 4, source.height() - 1); j++) {
			if (!omega(i, j)) {
				temp_conf = temp_conf + confidence_values(i, j);
			}
		}
	}
	p.conf = temp_conf / area;
	confidence_values(p.x_loc, p.y_loc) = p.conf;
}

void init(int width, int height, int square_x, int square_y, int square_size) {
	confidence_values = CImg<float>(source.width(), source.height(), 1, 1, 1);
	omega = CImg<bool>(source.width(), source.height(), 1, 1, 0);
	int i, j;
	for (i = square_x; i < square_x + square_size; i++) {
		for (j = square_y; j < square_y + square_size; j++) {
			confidence_values(i, j) = 0;
			omega(i, j) = 1;
		}
	}

}

Vector2d getNormal(pixel_info &p) {
	Vector2d normal = Vector2d();
	int i = p.x_loc;
	int jtop = std::max(p.y_loc - 1, 0);
	int jbot = std::min(p.y_loc + 1, omega.height() - 1);
	float yGrad = 0;
	if (i - 1 >= 0) {
		yGrad -= omega(i - 1, jbot);
		yGrad += omega(i - 1, jtop);
	}
	yGrad -= 2 * omega(i, jbot);
	yGrad += 2 * omega(i, jtop);
	if (i + 1 < omega.height()) {
		yGrad -= omega(i + 1, jbot);
		yGrad += omega(i + 1, jtop);
	}
	int j = p.y_loc;
	int ileft = std::max(p.x_loc - 1, 0);
	int iright = std::min(p.x_loc + 1, omega.width() - 1);
	float xGrad = 0;
	if (j - 1 >= 0) {
		xGrad -= omega(ileft, j - 1);
		xGrad += omega(iright, j - 1);
	}
	xGrad -= 2 * omega(ileft, j);
	xGrad += 2 * omega(iright, j);
	if (j + 1 < omega.width()) {
		xGrad -= omega(ileft, j + 1);
		xGrad += omega(iright, j + 1);
	}
	float length = std::sqrt(yGrad * yGrad + xGrad * xGrad);
	normal.x = xGrad;
	normal.y = yGrad;
	normal.normalize();
	return normal;
}

Vector2d getGradient(pixel_info &p) {
	Vector2d gradient = Vector2d();
	int i = p.x_loc;
	int jtop = std::max(p.y_loc - 1, 0);
	int jbot = std::min(p.y_loc + 1, omega.height() - 1);
	float yGrad = 0;
	int topWeight;
	int botWeight;
	if (i - 1 >= 0) {
		if (!omega(i - 1, jtop)) {
			yGrad += source(i - 1, jtop);
		}
		yGrad -= omega(i - 1, jbot);
		yGrad += omega(i - 1, jtop);
	}
	return gradient;
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

		// testing normal
		for (int i = squareleft; i < squareleft + squaresize; i++) {
			for (int j = squaretop; j < squaretop + squaresize; j++) {
				pixel_info p = {i, j, 0, 0, 0};
				getNormal(p);
			}
		}
		confidence_values.display();
		source.display();
	} else {
		printf("You must parameters like so: /imagequilt [in file] [square size]\n");
		return 0;
	}

	return 0;
}