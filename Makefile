SDL=`pkg-config --cflags --libs sdl2`
GLEW=`pkg-config --cflags --libs glew`
FFTW=`pkg-config --cflags --libs fftw3 fftw3f`
LIBAV=`pkg-config --cflags --libs libavcodec libavutil libswscale libswresample libavformat`

main:
	clang -w src/main.c -o bin/infra -lm $(SDL) $(GLEW) $(FFTW) $(LIBAV)
	./bin/infra

debug:
	gcc -w src/main.c -o bin/infra -lm $(SDL) $(GLEW) $(FFTW) $(LIBAV) -O0
	gdb ./bin/infra

build:
	gcc -w src/main.c -o bin/infra -lm $(SDL) $(GLEW) $(FFTW) $(LIBAV) -O3

run:
	./bin/infra
