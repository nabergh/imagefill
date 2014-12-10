main:
	g++ -o imagefill imagefill.cpp -lpthread -lX11 -g

windows:
	g++ -o imagefill imagefill.cpp -lpthread -lgdi32

makematte:
	g++ -o makematte makematte.cpp -lpthread -lX11 -g

clean:
	/bin/rm -f makematte
