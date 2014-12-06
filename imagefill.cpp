#include "CImg.h"
#include "math.h"
#include <limits>
#include <vector>
#include <algorithm>
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

bool comparePriority(pixel_info &p1, pixel_info &p2) {
	return p1.priority < p2.priority;
}

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
	result = CImg<unsigned char>(source);
	omega = CImg<bool>(source.width(), source.height(), 1, 1, 0);
	int i, j;
	for (i = square_x; i < square_x + square_size; i++) {
		for (j = square_y; j < square_y + square_size; j++) {
			confidence_values(i, j) = 0;
			omega(i, j) = 1;
			if (i == square_x || i == square_x + square_size - 1 || j == square_y || j == square_y + square_size - 1) {
				pixel_info d_sigma;
				d_sigma.x_loc = i;
				d_sigma.y_loc = j;
				d_sigma.conf = 1;
				d_sigma.data = 0;
				d_sigma.priority = d_sigma.conf * d_sigma.data;
				fillfront.push_back(d_sigma);
			}
		}
	}

}

pixel_info get_priority() {
	std::make_heap(fillfront.begin(), fillfront.end(), comparePriority);
	pixel_info p = fillfront.front();
	std::pop_heap(fillfront.begin(), fillfront.end(), comparePriority);
	fillfront.pop_back();
	return p;
}
float SSD(pixel_info p, int qx, int qy) {
	int i, j;
	float sum = 0;
	for (i = std::max(p.x_loc - 4, 0); i <= std::min(p.x_loc + 4, source.width() - 1); i++) {
		for (j = std::max(p.y_loc - 4, 0); j <= std::min(p.y_loc + 4, source.height() - 1); j++) {
			if (!omega(i, j)) {
				sum = sum + pow(source(i, j, 0, 0) - source(qx + i - p.x_loc, qy + j - p.y_loc, 0, 0), 2) +
				      pow(source(i, j, 0, 1) - source(qx + i - p.x_loc, qy + j - p.y_loc, 0, 1), 2) +
				      pow(source(i, j, 0, 2) - source(qx + i - p.x_loc, qy + j - p.y_loc, 0, 2), 2);
			}
		}
	}
	return sum;
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
	if (i + 1 < omega.width()) {
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
	if (j + 1 < omega.height()) {
		xGrad -= omega(ileft, j + 1);
		xGrad += omega(iright, j + 1);
	}
	float length = std::sqrt(yGrad * yGrad + xGrad * xGrad);
	normal.x = xGrad;
	normal.y = yGrad;
	normal.normalize();
	return normal;
}


void searchBorder(int width, int x_loc, int y_loc, pixel_info &nearest) {
	int x, y, xmin, xmax, ymin, ymax;
	x = x_loc;
	y = fmax(y_loc + width / 2, 0.0);
	xmax = fmin(x + width / 2, source.width() - 1);
	ymax = fmin(y + width / 2, source.height() - 1);
	xmin = fmax(x - width / 2, 0.0);
	xmax = fmax(y - width / 2, 0.0);
	for (; x <= xmax; x++) {
		if (!omega(x, y)) {
			nearest.x_loc = x;
			nearest.y_loc = y;
			return;
		}
	}
	for (; y <= ymax; y++) {
		if (!omega(x, y)) {
			nearest.x_loc = x;
			nearest.y_loc = y;
			return;
		}
	}
	for (; x >= xmin; x--) {
		if (!omega(x, y)) {
			nearest.x_loc = x;
			nearest.y_loc = y;
			return;
		}
	}
	for (; y >= ymin; y--) {
		if (!omega(x, y)) {
			nearest.x_loc = x;
			nearest.y_loc = y;
			return;
		}
	}
	for (; x < x_loc; x++) {
		if (!omega(x, y)) {
			nearest.x_loc = x;
			nearest.y_loc = y;
			return;
		}
	}
}

pixel_info findNearest(int x, int y) {
	int width = 3;
	pixel_info nearest = { -1, -1, 0, 0, 0};
	while (nearest.x_loc == -1) {
		searchBorder(width, x, y, nearest);
		width += 2;
	}
	return nearest;
}

Vector2d getGradient(pixel_info &p) {
	Vector2d gradient = Vector2d();
	pixel_info nearest;
	int c;
	float topGrad, botGrad;
	topGrad = botGrad = 0;
	int jtop = std::max(p.y_loc - 1, 0);
	int jbot = std::min(p.y_loc + 1, source.height() - 1);
	int i = p.x_loc - 1;
	if (i >= 0) {
		if (omega(i, jtop)) {
			nearest = findNearest(i, jtop);
			cimg_forC(source, c) {
				topGrad += source(nearest.x_loc, nearest.y_loc, c);
			}
		} else {
			cimg_forC(source, c) {
				topGrad += source(i, jtop, c);
			}
		}
		if (omega(i, jbot)) {
			nearest = findNearest(i, jbot);
			cimg_forC(source, c) {
				botGrad += source(nearest.x_loc, nearest.y_loc, c);
			}
		} else {
			cimg_forC(source, c) {
				botGrad += source(i, jbot, c);
			}
		}
	}
	i++;
	if (omega(i, jtop)) {
		nearest = findNearest(i, jtop);
		cimg_forC(source, c) {
			topGrad += 2 * source(nearest.x_loc, nearest.y_loc, c);
		}
	} else {
		cimg_forC(source, c) {
			topGrad += 2 * source(i, jtop, c);
		}
	}
	if (omega(i, jbot)) {
		nearest = findNearest(i, jbot);
		cimg_forC(source, c) {
			botGrad += 2 * source(nearest.x_loc, nearest.y_loc, c);
		}
	} else {
		cimg_forC(source, c) {
			botGrad += 2 * source(i, jbot, c);
		}
	}
	i++;
	if (i < source.width()) {
		if (omega(i, jtop)) {
			nearest = findNearest(i, jtop);
			cimg_forC(source, c) {
				topGrad += source(nearest.x_loc, nearest.y_loc, c);
			}
		} else {
			cimg_forC(source, c) {
				topGrad += source(i, jtop, c);
			}
		}
		if (omega(i, jbot)) {
			nearest = findNearest(i, jbot);
			cimg_forC(source, c) {
				botGrad += source(nearest.x_loc, nearest.y_loc, c);
			}
		} else {
			cimg_forC(source, c) {
				botGrad += source(i, jbot, c);
			}
		}
	}
	float leftGrad, rightGrad;
	leftGrad = rightGrad = 0;
	int ileft = std::max(p.x_loc - 1, 0);
	int iright = std::min(p.x_loc + 1, source.width() - 1);
	int j = p.y_loc - 1;
	if (j >= 0) {
		if (omega(iright, j)) {
			nearest = findNearest(iright, j);
			cimg_forC(source, c) {
				rightGrad += source(nearest.x_loc, nearest.y_loc, c);
			}
		} else {
			cimg_forC(source, c) {
				rightGrad += source(iright, j, c);
			}
		}
		if (omega(ileft, j)) {
			nearest = findNearest(ileft, j);
			cimg_forC(source, c) {
				leftGrad += source(nearest.x_loc, nearest.y_loc, c);
			}
		} else {
			cimg_forC(source, c) {
				leftGrad += source(ileft, j, c);
			}
		}
	}
	j++;
	if (omega(iright, j)) {
		nearest = findNearest(iright, j);
		cimg_forC(source, c) {
			rightGrad += 2 * source(nearest.x_loc, nearest.y_loc, c);
		}
	} else {
		cimg_forC(source, c) {
			rightGrad += 2 * source(iright, j, c);
		}
	}
	if (omega(ileft, j)) {
		nearest = findNearest(ileft, j);
		cimg_forC(source, c) {
			leftGrad += 2 * source(nearest.x_loc, nearest.y_loc, c);
		}
	} else {
		cimg_forC(source, c) {
			leftGrad += 2 * source(ileft, j, c);
		}
	}
	j++;
	if (j < source.height()) {
		if (omega(iright, j)) {
			nearest = findNearest(iright, j);
			cimg_forC(source, c) {
				rightGrad += source(nearest.x_loc, nearest.y_loc, c);
			}
		} else {
			cimg_forC(source, c) {
				rightGrad += source(iright, j, c);
			}
		}
		if (omega(ileft, j)) {
			nearest = findNearest(ileft, j);
			cimg_forC(source, c) {
				leftGrad += source(nearest.x_loc, nearest.y_loc, c);
			}
		} else {
			cimg_forC(source, c) {
				leftGrad += source(ileft, j, c);
			}
		}
	}

	gradient.x = rightGrad - leftGrad;
	gradient.y = botGrad - topGrad;
	gradient.normalize();
	return gradient;
}

Vector2d getGradient_old(pixel_info &p) {
	Vector2d gradient = Vector2d();
	int i = p.x_loc - 1;
	int jtop = std::max(p.y_loc - 1, 0);
	int jbot = std::min(p.y_loc + 1, source.height() - 1);
	float topGrad, botGrad;
	topGrad = botGrad = 0;
	int topWeight, botWeight;
	topWeight = botWeight = 0;
	int c;
	if (i >= 0) {
		if (!omega(i, jtop)) {
			cimg_forC(source, c) {
				topGrad += source(i, jtop, c);
			}
			topWeight++;
		}
		if (!omega(i, jbot)) {
			cimg_forC(source, c) {
				botGrad -= source(i, jbot, c);
			}
			botWeight++;
		}
	}
	i++;
	if (!omega(i, jtop)) {
		cimg_forC(source, c) {
			topGrad += 2 * source(i, jtop, c);
		}
		topWeight += 2;
	}
	if (!omega(i, jbot)) {
		cimg_forC(source, c) {
			botGrad -= 2 * source(i, jbot, c);
		}
		botWeight += 2;
	}
	i++;
	if (i < source.width()) {
		if (!omega(i, jtop)) {
			cimg_forC(source, c) {
				topGrad += source(i, jtop, c);
			}
			topWeight++;
		}
		if (!omega(i, jbot)) {
			cimg_forC(source, c) {
				botGrad -= source(i, jbot, c);
			}
			botWeight++;
		}
	}
	int j = p.y_loc - 1;
	int ileft = std::max(p.x_loc - 1, 0);
	int iright = std::min(p.x_loc + 1, source.width() - 1);
	float leftGrad, rightGrad;
	leftGrad = rightGrad = 0;
	int leftWeight, rightWeight;
	leftWeight = rightWeight = 0;
	if (j >= 0) {
		if (!omega(iright, j)) {
			cimg_forC(source, c) {
				rightGrad += source(iright, j, c);
			}
			rightWeight++;
		}
		if (!omega(ileft, j)) {
			cimg_forC(source, c) {
				leftGrad -= source(ileft, j, c);
			}
			leftWeight++;
		}
	}
	j++;
	if (!omega(iright, j)) {
		cimg_forC(source, c) {
			rightGrad += 2 * source(iright, j, c);
		}
		rightWeight += 2;
	}
	if (!omega(ileft, j)) {
		cimg_forC(source, c) {
			leftGrad -= 2 * source(ileft, j, c);
		}
		leftWeight += 2;
	}
	j++;
	if (j < source.height()) {
		if (!omega(iright, j)) {
			cimg_forC(source, c) {
				rightGrad += source(iright, j, c);
			}
			rightWeight++;
		}
		if (!omega(ileft, j)) {
			cimg_forC(source, c) {
				leftGrad -= source(ileft, j, c);
			}
			leftWeight++;
		}
	}

	float yWeight = topWeight / botWeight;
	botGrad *= yWeight;
	float xWeight = rightWeight / leftWeight;
	leftGrad *= xWeight;
	gradient.x = leftGrad + rightGrad;
	gradient.y = topGrad + botGrad;
	gradient.x *= (topWeight / rightWeight);
	gradient.normalize();
	return gradient;
}

void inpaint() {
	while (!fillfront.empty()) {

		//compute priorities

		//get next patch to fill
		pixel_info next = get_priority();

		//get minimum patch
		pixel_info min_patch;
		float min = std::numeric_limits<float>::max();
		for (int i = 4; i < source.width() - 4; i++) {
			for (int j = 4; j < source.height() - 4; j++) {
				float temp = SSD(next, i, j);
				if (temp < min) {
					min_patch.x_loc = i;
					min_patch.y_loc = j;
					min = temp;
				}
			}
		}

		//fill
		for (int i = std::max(next.x_loc - 4, 0); i <= std::min(next.x_loc + 4, source.width() - 1); i++) {
			for (int j = std::max(next.y_loc - 4, 0); j <= std::min(next.y_loc + 4, source.height() - 1); j++) {
				if (omega(i, j)) {
					source(i, j, 0, 0) = source(min_patch.x_loc + i - next.x_loc, min_patch.y_loc + j - next.y_loc, 0, 0);
					source(i, j, 0, 1) = source(min_patch.x_loc + i - next.x_loc, min_patch.y_loc + j - next.y_loc, 0, 1);
					source(i, j, 0, 2) = source(min_patch.x_loc + i - next.x_loc, min_patch.y_loc + j - next.y_loc, 0, 2);
				}
			}
		}

		//recalculate confidences
		confidence(next);

		//add to fillfront
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

		// testing normal
		for (int i = squareleft; i < squareleft + squaresize; i++) {
			for (int j = squaretop; j < squaretop + squaresize; j++) {
				pixel_info p = {i, j, 0, 0, 0};
				getNormal(p);
				// printf("%f %f %f\n", source(0, 0, 1));
				getGradient(p);
			}
		}
		confidence_values.display();
		source.display();
	} else {
		printf("You must input parameters like so: /imagequilt [in file] [square size]\n");
		return 0;
	}

	return 0;
}
