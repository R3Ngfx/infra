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

#include "tinydir/tinydir.h"

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024

#define pi 3.14159265358979323846
#define eps 0.000001

// Rendering variables
unsigned int viewportWidth = 1280, viewportHeight = 720;
unsigned int renderWidth = 1280, renderHeight = 720;
unsigned int newRenderWidth = 1280, newRenderHeight = 720;
unsigned int frameRate = 60;
unsigned int videoBitrate = 24000; // Kb/s
unsigned int audioBitrate = 128; // Kb/s
unsigned int outputSampleRate = 48000; // Hz

// Global context variables
SDL_GLContext context;
SDL_Window* window;
struct nk_context *ctx;

// Application behaviour variables
unsigned char playing = 0;
unsigned char reloadShaders = 0;
unsigned char hideUI = 0;
unsigned char saveFrame = 0;
unsigned char startVideo = 0;
unsigned char saveVideo = 0;
unsigned char changePauseAudio = 0;
unsigned char useAudio = 0; // TODO: Implement

// Time variables
unsigned long currentTimeSelect, prevTimeSelect;
double currentFrameTime = 0, prevFrameTime = 0, currentTime = 0, deltaTime;
unsigned int currentVideoFrame = 0, maxVideoFrames = 3600;
unsigned int currentAudioSample = 0;
float renderVideoLength = 60;
double lastRenderedTime = -1;

// Directory variables
tinydir_dir dir;
char videoFilename[256] = "out/video.mp4";
int videoFilenameLen = 13;
char shaderPath[256] = "data/shader.frag";
int shaderPathLen = 16;

// Audio variables
char trackPath[256] = "";
int trackPathLen = 0;
unsigned int trackSampleRate = 48000;
unsigned char trackChannels = 2;
char reloadTrack = 0;
float lows = 0, mids = 0, highs = 0;
float maxLows = eps, maxMids = eps, maxHighs = eps;
float normalizedLows = 0, normalizedMids = 0, normalizedHighs = 0;
float smoothness = 6, power = 1, drop = 0;

// Linear interpolation
float lerp(float a, float b, float x) {
	return (1-x)*a + x*b;
}

// Clamp value between min and max
float clamp(float min, float max, float x) {
	return (x < min ? min : (x > max ? max : x));
}

float min(float a, float b) {
	return a < b ? a : b;
}

float max(float a, float b) {
	return a > b ? a : b;
}

#endif
