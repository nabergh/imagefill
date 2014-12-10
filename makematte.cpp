#include "CImg.h"
using namespace cimg_library;

CImg<unsigned char> source;
CImg<unsigned char> matte;

int main(int argc, char *argv[]) {
	int brushwidth = 4;
	if (argc == 3) {
		source = CImg<unsigned char>(argv[1]);
		matte = CImg<unsigned char>(source.width(), source.height(), 1, 1, 0);
		CImgDisplay img_display(source, "Draw the matte outline"), matte_display(matte, "matte preview");
		img_display.move(0, 200);
		matte_display.move(source.width(), 200);
		while (!img_display.is_closed() && !matte_display.is_closed()) {
			int c;
			img_display.wait();
			if (img_display.button()) {
				if (img_display.mouse_x() >= 0 && img_display.mouse_x() < source.width() && img_display.mouse_y() >= 0 && img_display.mouse_y() < source.height()) {
					for (int i = std::max(img_display.mouse_x() - brushwidth, 0); i < std::min(img_display.mouse_x() + brushwidth, source.width()); ++i) {
						for (int j = std::max(img_display.mouse_y() - brushwidth, 0); j < std::min(img_display.mouse_y() + brushwidth, source.height()); ++j) {
							cimg_forC(source, c) {
								source(i, j, c) = 255;
							}
							matte(i, j) = 255;
						}
					}
					source.display(img_display);
					matte.display(matte_display);
				}
			}
		}
		matte.save(argv[2]);
	} else {
		printf("You must input parameters like so: /imagequilt [in file] [out file]\n");
	}

	return 0;
}