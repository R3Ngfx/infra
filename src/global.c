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
int renderWidth = 1280, renderHeight = 720;
int viewportWidth = 1280, viewportHeight = 720;
struct nk_context *ctx;
SDL_GLContext context;
double currFrameTime = 0, prevFrameTime = 0, deltaTime;
SDL_Window* window;

// Linear interpolation
double lerp(double a, double b, double x) {
	return (1-x)*a + x*b;
}

#endif
