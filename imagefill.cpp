#include "CImg.h"
#include "math.h"
#include <limits>
#include <vector>
#include <algorithm>
#include <cfloat>
using namespace cimg_library;


CImg<unsigned char> source;
CImg<unsigned char> matte;
CImg<unsigned char> result;
CImg<float> confidence_values;
CImg<bool> front;
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
		if (length > 0) {
			x /= length;
			y /= length;
		}
	};
	float operator*(Vector2d v1) {
		return x * v1.x + y * v1.y;
	};
	void operator/=(float d) {
		x /= d;
		y /= d;
	};
	bool isZero() {
		return x == 0 && y == 0;
	}
};

float confidence(pixel_info &p) {
	int i, j;
	float temp_conf = 0;
	int area = (std::min(p.x_loc + 4, source.width() - 1) - std::max(p.x_loc - 4, 0)) * (std::min(p.y_loc + 4, source.height() - 1) - std::max(p.y_loc - 4, 0));
	for (i = std::max(p.x_loc - 4, 0); i <= std::min(p.x_loc + 4, source.width() - 1); i++) {
		for (j = std::max(p.y_loc - 4, 0); j <= std::min(p.y_loc + 4, source.height() - 1); j++) {
			if (!omega(i, j)) {
				temp_conf = temp_conf + confidence_values(i, j);
			}
		}
	}
	p.conf = temp_conf / area;
	confidence_values(p.x_loc, p.y_loc) = p.conf;
	return p.conf;
}

void init(int width, int height, CImg<unsigned char> matte) {
	confidence_values = CImg<float>(source.width(), source.height(), 1, 1, 1);
	result = CImg<unsigned char>(source);
	omega = CImg<bool>(source.width(), source.height(), 1, 1, 0);
	front = CImg<bool>(source.width(), source.height(), 1, 1, 0);
	int i, j;
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			if (matte(i, j) == 255) {
				confidence_values(i, j) = 0;
				omega(i, j) = 1;
				if ( (i > 0 && matte(i-1, j) != 255) || (i < width-1 && matte(i+1, j) != 255) || (j > 0 && matte(i, j-1) != 255) || (j < height-1 && matte(i,j+1) != 255) ) {
					pixel_info d_sigma;
					d_sigma.x_loc = i;
					d_sigma.y_loc = j;
					d_sigma.conf = 0;
					d_sigma.data = 0;
					d_sigma.priority = d_sigma.conf * d_sigma.data;
					fillfront.push_back(d_sigma);
					front(i, j) = 1;

				}
			}
		}
	}

}

pixel_info get_priority() {
	std::make_heap(fillfront.begin(), fillfront.end(), comparePriority);
	pixel_info p = fillfront.front();
	std::pop_heap(fillfront.begin(), fillfront.end(), comparePriority);
	fillfront.pop_back();
	front(p.x_loc, p.y_loc) = 0;
	return p;
}
float SSD(pixel_info p, int qx, int qy) {
	int i, j;
	float sum = 0;
	for (i = std::max(p.x_loc - 4, 0); i <= std::min(p.x_loc + 4, source.width() - 1); i++) {
		for (j = std::max(p.y_loc - 4, 0); j <= std::min(p.y_loc + 4, source.height() - 1); j++) {
			if (!omega(qx, qy)) {
				if (!omega(i, j)) {
					sum = sum + pow(source(i, j, 0, 0) - source(qx + i - p.x_loc, qy + j - p.y_loc, 0, 0), 2) +
					      pow(source(i, j, 0, 1) - source(qx + i - p.x_loc, qy + j - p.y_loc, 0, 1), 2) +
					      pow(source(i, j, 0, 2) - source(qx + i - p.x_loc, qy + j - p.y_loc, 0, 2), 2);
				} else {
					return FLT_MAX;
				}
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
		yGrad += omega(i - 1, jbot);
		yGrad -= omega(i - 1, jtop);
	}
	yGrad += 2 * omega(i, jbot);
	yGrad -= 2 * omega(i, jtop);
	if (i + 1 < omega.width()) {
		yGrad += omega(i + 1, jbot);
		yGrad -= omega(i + 1, jtop);
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
	normal.x = xGrad;
	normal.y = yGrad;
	normal.normalize();
	return normal;
}

pixel_info directionalSearch(Vector2d dir, int x_loc, int y_loc) {
	pixel_info nearest = { -1, -1, 0, 0, 0};
	float x, y;
	x = x_loc + 0.5;
	y = y_loc + 0.5;
	x += dir.x;
	y += dir.y;
	nearest.x_loc = (int) x;
	nearest.y_loc = (int) y;
	while (nearest.x_loc >= 0 && nearest.y_loc >= 0 && nearest.x_loc < source.width() && nearest.y_loc < source.height()) {
		if (!(omega(nearest.x_loc, nearest.y_loc))) {
			return nearest;
		}
		x += dir.x;
		y += dir.y;
		nearest.x_loc = (int) x;
		nearest.y_loc = (int) y;
	}
}

float luminance(int x, int y) {
	float l;
	if (source.spectrum() == 3) {
		l = 0.2126 * source(x, y, 0, 0);
		l += 0.7152 * source(x, y, 0, 1);
		l += 0.0722 * source(x, y, 0, 2);
	} else {
		l = source(x, y, 0, 0);
	}
	return l;
}

Vector2d getGradient(pixel_info &p) {
	Vector2d gradient = Vector2d();
	Vector2d normal = getNormal(p);
	if (normal.isZero()) {
		return gradient;
	}
	normal.x = normal.x * -1;
	normal.y = normal.y * -1;
	pixel_info nearest;
	int c;
	float topGrad, botGrad;
	topGrad = botGrad = 0;
	int jtop = std::max(p.y_loc - 1, 0);
	int jbot = std::min(p.y_loc + 1, source.height() - 1);
	int i = p.x_loc - 1;
	if (i >= 0) {
		if (omega(i, jtop)) {
			nearest = directionalSearch(normal, i, jtop);
			topGrad += luminance(nearest.x_loc, nearest.y_loc);
		} else {
			topGrad += luminance(i, jtop);
		}
		if (omega(i, jbot)) {
			nearest = directionalSearch(normal, i, jbot);
			botGrad += luminance(nearest.x_loc, nearest.y_loc);
		} else {
			botGrad += luminance(i, jbot);
		}
	}
	i++;
	if (omega(i, jtop)) {
		nearest = directionalSearch(normal, i, jtop);
		topGrad += 2 * luminance(nearest.x_loc, nearest.y_loc);
	} else {
		topGrad += 2 * luminance(i, jtop);
	}
	if (omega(i, jbot)) {
		nearest = directionalSearch(normal, i, jbot);
		botGrad += 2 * luminance(nearest.x_loc, nearest.y_loc);
	} else {
		botGrad += 2 * luminance(i, jbot);
	}
	i++;
	if (i < source.width()) {
		if (omega(i, jtop)) {
			nearest = directionalSearch(normal, i, jtop);
			topGrad += luminance(nearest.x_loc, nearest.y_loc);
		} else {
			topGrad += luminance(i, jtop);
		}
		if (omega(i, jbot)) {
			nearest = directionalSearch(normal, i, jbot);
			botGrad += luminance(nearest.x_loc, nearest.y_loc);
		} else {
			botGrad += luminance(i, jbot);
		}
	}
	float leftGrad, rightGrad;
	leftGrad = rightGrad = 0;
	int ileft = std::max(p.x_loc - 1, 0);
	int iright = std::min(p.x_loc + 1, source.width() - 1);
	int j = p.y_loc - 1;
	if (j >= 0) {
		if (omega(iright, j)) {
			nearest = directionalSearch(normal, iright, j);
			rightGrad += luminance(nearest.x_loc, nearest.y_loc);
		} else {
			rightGrad += luminance(iright, j);
		}
		if (omega(ileft, j)) {
			nearest = directionalSearch(normal, ileft, j);
			leftGrad += luminance(nearest.x_loc, nearest.y_loc);
		} else {
			leftGrad += luminance(ileft, j);
		}
	}
	j++;
	if (omega(iright, j)) {
		nearest = directionalSearch(normal, iright, j);
		rightGrad += 2 * luminance(nearest.x_loc, nearest.y_loc);
	} else {
		rightGrad += 2 * luminance(iright, j);
	}
	if (omega(ileft, j)) {
		nearest = directionalSearch(normal, ileft, j);
		leftGrad += 2 * luminance(nearest.x_loc, nearest.y_loc);
	} else {
		leftGrad += 2 * luminance(ileft, j);
	}
	j++;
	if (j < source.height()) {
		if (omega(iright, j)) {
			nearest = directionalSearch(normal, iright, j);
			rightGrad += luminance(nearest.x_loc, nearest.y_loc);
		} else {
			rightGrad += luminance(iright, j);
		}
		if (omega(ileft, j)) {
			nearest = directionalSearch(normal, ileft, j);
			leftGrad += luminance(nearest.x_loc, nearest.y_loc);
		} else {
			leftGrad += luminance(ileft, j);
		}
	}
	gradient.y = rightGrad - leftGrad;
	gradient.x = botGrad - topGrad;
	gradient /= 1020;
	return gradient;
}

float data(pixel_info &p) {
	return fabs(getGradient(p) * getNormal(p));
}

void inpaint() {
	while (!fillfront.empty()) {	
	    //compute priorities
	 	for (int i = 0; i < fillfront.size(); i++) {
	 		fillfront[i].conf = confidence(fillfront[i]);
	 		fillfront[i].data = data(fillfront[i]);
	 		fillfront[i].priority = fillfront[i].conf * fillfront[i].data;
	 	}

		//get next patch to fill
		pixel_info next = get_priority();

	     //get minimum patch
	     pixel_info min_patch;
	     float min = std::numeric_limits<float>::max();
	     for (int i = 4; i < source.width() - 4; i++) {
	         for (int j = 4; j < source.height() - 4; j++) {
	         	if (!omega(i, j)) {
	             	float temp = SSD(next, i, j);
		             if (temp < min) {
		                 min_patch.x_loc = i;
		                 min_patch.y_loc = j;
		                 min = temp;
		             }
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
	                 omega(i, j) = 0;
	                 front(i, j) = 0;
	                 confidence_values(i, j) = next.conf;
	             }
	         }
	     }

	     source.display();

	     //add to fillfront
	     for (int i = std::max(next.x_loc - 4, 0); i <= std::min(next.x_loc + 4, source.width() - 1); i++) {
	         for (int j = std::max(next.y_loc - 4, 0); j <= std::min(next.y_loc + 4, source.height() - 1); j++) {
	             if (i == std::max(next.x_loc - 4, 0) || i == std::min(next.x_loc + 4, source.width() - 1) ||
	                     j == std::max(next.y_loc - 4, 0) || j == std::min(next.y_loc + 4, source.height() - 1)) {
	                 pixel_info d_sigma;
	                 d_sigma.x_loc = i;
	                 d_sigma.y_loc = j;
	                 if (!front(i, j)) {
	                  	fillfront.push_back(d_sigma);
	                  	front(i, j) = 1;
	                 } 
	             }
	         }
	     }
	}
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("You must input parameters like so: /imagequilt [in file] [matte]\n");
		return 0;
	}

	source = CImg<unsigned char>(argv[1]);

	init(source.width(), source.height(), CImg<unsigned char>(argv[2]));
	inpaint();

	source.display();

	return 0;
}
