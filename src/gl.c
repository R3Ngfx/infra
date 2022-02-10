// OpenGL functions and handling
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"

// Vertex shader
const char* vertSource = "#version 400 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"void main() {\n"
		"gl_Position = vec4(aPos, 1.0);\n"
	"}\0";
char* fragSource = NULL;

// Initialize GL
int initGL() {

	// Initialize SDL
	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	window = SDL_CreateWindow("Infra", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, viewportWidth, viewportHeight,
		SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);
	context = SDL_GL_CreateContext(window);
	if (context == NULL) {
		printf("Error initializing SDL\n");
		return 0;
	}

	// Initialize GLEW
	glViewport(0, 0, viewportWidth, viewportHeight);
	glewExperimental = 1;
	if (glewInit() != GLEW_OK) {
		printf("Error initializing GLEW\n");
		return 0;
	}
	return 1;

	//V-Sync
	SDL_GL_SetSwapInterval(1);
}

// Uninitialize GL
void uninitGL() {
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
}

// Loads fragment shader source from file
int loadFragment(char* path) {
	if (fragSource != NULL) {
		free(fragSource);
	}
	FILE* f = fopen(path, "r");
	if (f == NULL) {
		printf("Error when reading file %s\n", path);
		return 0;
	}
	fseek (f, 0, SEEK_END);
	long len = ftell(f);
	fseek (f, 0, SEEK_SET);
	fragSource = malloc(len);
	fread(fragSource, 1, len, f);
	fclose(f);
	return 1;
}

// Initialize shaders
void initShader(char* path) {
	// TODO
}

// Render graphics
void renderGL(){
	SDL_GetWindowSize(window, &viewportWidth, &viewportHeight);
	glViewport(0, 0, viewportWidth, viewportHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
	SDL_GL_SwapWindow(window);
}
