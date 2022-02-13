infra:
	gcc -w src/main.c -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -o bin/infra
	./bin/infra

build:
	gcc -w src/main.c -O3 -lm -lSDL2 -lGL -lGLEW -lpthread -ldl -lfftw3 -o bin/infra

run:
	./bin/infra
