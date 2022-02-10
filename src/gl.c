// OpenGL functions and handling
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"

// Global shader variables
GLuint shaderProgram = 0;
GLuint vertShader, fragShader;
GLint success = GL_FALSE;
GLuint vbo, vao;

// Vertex quad
float vertices[] = {
	-1.0f, -1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,
	-1.0f, 1.0f, 0.0f,
	1.0f, 1.0f, 0.0f
};

// Shader source
char* fragSource = NULL;
const char* vertSource = "#version 400 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"void main() {\n"
		"gl_Position = vec4(aPos, 1.0);\n"
	"}\0";

// Check shader compilation is correct
int checkShader(GLuint shader){
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("Error while compiling shader:\n%s", infoLog);
		return 0;
	}
	return 1;
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

// Assign shader variables
void setShaderVariables() {
	int timeLoc = glGetUniformLocation(shaderProgram, "time");
	glUniform1f(timeLoc, currFrameTime);
	int screenResLoc = glGetUniformLocation(shaderProgram, "resolution");
	glUniform2f(screenResLoc, viewportWidth, viewportHeight); // TODO: CHANGE TO RENDER SIZE
}

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
	SDL_GL_SetSwapInterval(1);
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

	// Config OpenGL shaders
	shaderProgram = glCreateProgram();
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertSource, NULL);
	glCompileShader(vertShader);
	if (!checkShader(vertShader)) return 0;
	glAttachShader(shaderProgram, vertShader);
	loadFragment("data/shader.frag");
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, NULL);
	glCompileShader(fragShader);
	if (!checkShader(fragShader)) return 0;
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		printf("Error while linking shaders:\n");
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s", infoLog);
	}
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(vao);

	return 1;
}

// Render graphics
void renderGL(){
	SDL_GetWindowSize(window, &viewportWidth, &viewportHeight);
	glViewport(0, 0, viewportWidth, viewportHeight);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vao);
	glBindVertexArray(vbo);
	glUseProgram(shaderProgram);
	setShaderVariables();
	glDrawArrays(GL_TRIANGLES, 0, 6);
	nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
	SDL_GL_SwapWindow(window);
}

// Uninitialize GL
void uninitGL() {
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
}
