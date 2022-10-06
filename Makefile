debug:
	gcc -w src/main.c -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -lavcodec -lavutil -lswscale -o bin/infra
	./bin/infra

build:
	gcc -w src/main.c -O3 -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -lavcodec -lavutil -lswscale -o bin/infra

run:
	./bin/infra

video:
	ffmpeg -r 60 -i "out/frame%04d.bmp" -vf vflip "out.mp4"
