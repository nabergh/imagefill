main:
	g++ -o imagefill imagefill.cpp -lpthread -lX11 -g

windows:
	g++ -o imagefill imagefill.cpp -lpthread -lgdi32
