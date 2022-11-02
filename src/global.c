// Global functions and variables
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#ifndef GLOBAL
#define GLOBAL

#include <stdio.h>

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_gl3.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

// Global variables
unsigned int viewportWidth = 1280, viewportHeight = 720;
unsigned int renderWidth = 1280, renderHeight = 720;
int newRenderWidth = 1280, newRenderHeight = 720;
int frameRate = 60;
int bitrate = 24000000;
struct nk_context *ctx;
SDL_GLContext context;
unsigned long currentTimeSelect, prevTimeSelect;
double currentFrameTime = 0, prevFrameTime = 0, time = 0, deltaTime;
SDL_Window* window;
unsigned int saveFrame = 0, startVideo = 0, saveVideo = 0;
unsigned int currentVideoFrame = 0, maxVideoFrames = 3600;
int renderVideoLength = 60;
int playing = 1;

// Linear interpolation
double lerp(double a, double b, double x) {
	return (1-x)*a + x*b;
}

// Load file contents to string
char* loadString(char* path) {
	SDL_RWops* f = SDL_RWFromFile(path, "r");
	if (f == NULL) {
		printf("Error when reading file %s\n", path);
		return NULL;
	}
	Sint64 len = SDL_RWseek(f, 0, SEEK_END);
	SDL_RWseek(f, 0, SEEK_SET);
	char* ret = malloc(len);
	SDL_RWread(f, ret, len, 1);
	SDL_RWclose(f);
	return ret;
}

#endif
