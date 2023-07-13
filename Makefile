main:
	gcc -w src/main.c -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -lfftw3f -lavcodec -lavutil -lswscale -lswresample -lavformat -o bin/infra
	./bin/infra

debug:
	gcc -w src/main.c -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -lfftw3f -lavcodec -lavutil -lswscale -lswresample -lavformat -o bin/infra -O0 -g3
	gdb ./bin/infra

build:
	gcc -w src/main.c -O3 -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -lfftw3f -lavcodec -lavutil -lswscale -lswresample -lavformat -o bin/infra

run:
	./bin/infra

video:
	ffmpeg -r 60 -i "out/frame%04d.bmp" -vf vflip "out.mp4"
