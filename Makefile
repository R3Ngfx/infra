infra:
	gcc src/main.c -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -o bin/infra

build:
	gcc src/main.c -O3 -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -o bin/infra

run:
	./bin/infra
