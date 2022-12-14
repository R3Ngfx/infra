// SDL & OpenGL rendering functions and handling
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"
#include "export.c"

int renderResLoc;
int viewportResLoc;
// Global shader variables
GLuint vao, vbo;
GLuint fbo;
GLuint shaderProgram, viewportShaderProgram;
GLuint renderTexture;
GLint success = GL_FALSE;

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
const char* vertSource = "#version 330 core\n"
	"layout (location = 0) in vec3 pos;\n"
	"void main() {\n"
		"gl_Position = vec4(pos, 1.0);\n"
	"}\0";
const char* viewportFragSource = "#version 330 core\n"
	"uniform vec2 resolution;\n"
	"uniform sampler2D renderedTexture;\n"
	"out vec4 outCol;\n"
	"void main(void) {\n"
		"vec2 uv = gl_FragCoord.xy/resolution.xy;\n"
		"uv.y = 1-uv.y;\n"
		"outCol = texture(renderedTexture, uv);\n"
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
	fragSource = loadString(path);
	if (fragSource == NULL) {
		return 1;
	}
	return 0;
}

// Set up FBO
int setFBO() {
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(1, &renderTexture);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderWidth, renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Error while setting up FBO\n");
		return 0;
	}
	return 1;
}

// Set up render and viewport shaders
int setShaders() {
	// Set up render shaders
	GLuint vertShader, fragShader;
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
		printf("Error while linking render shaders:\n");
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("%s", infoLog);
		return 0;
	}
	glDeleteShader(fragShader);

	// Set up viewport shaders
	viewportShaderProgram = glCreateProgram();
	glAttachShader(viewportShaderProgram, vertShader);
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &viewportFragSource, NULL);
	glCompileShader(fragShader);
	if (!checkShader(fragShader)) return 0;
	glAttachShader(viewportShaderProgram, fragShader);
	glLinkProgram(viewportShaderProgram);
	glGetProgramiv(viewportShaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		printf("Error while linking viewport shaders:\n");
		char infoLog[512];
		glGetProgramInfoLog(viewportShaderProgram, 512, NULL, infoLog);
		printf("%s", infoLog);
		return 0;
	}
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	lastRenderedTime = -1;

	return 1;
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
		SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI|SDL_WINDOW_RESIZABLE);
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

	// Set up VAO and VBO
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(vao);

	// Set up FBO
	if (!setFBO()) {
		return 0;
	}

	// Set up render and viewport shaders
	if (!setShaders()) {
		return 0;
	}

	return 1;

}

// Render graphics
void renderGL(){

	SDL_GetWindowSize(window, (int*)&viewportWidth, (int*)&viewportHeight);

	// Render
	if (lastRenderedTime != currentTime || saveFrame || saveVideo) {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glViewport(0, 0, renderWidth, renderHeight);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(vao);
		glBindVertexArray(vbo);
		glUseProgram(shaderProgram);
		int timeLoc = glGetUniformLocation(shaderProgram, "time");
		glUniform1f(timeLoc, currentTime);
		renderResLoc = glGetUniformLocation(shaderProgram, "resolution");
		glUniform2f(renderResLoc, renderWidth, renderHeight);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		lastRenderedTime = currentTime;
	}

	// Save frame
	if (saveFrame) {
		glFinish();
		exportFrame();
	}

	// Save render
	if (saveVideo) {
		glFinish();
		encodeVideoFrame();
	}

	// Viewport
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, viewportWidth, viewportHeight);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vao);
	glBindVertexArray(vbo);
	glUseProgram(viewportShaderProgram);
	viewportResLoc = glGetUniformLocation(viewportShaderProgram, "resolution");
	glUniform2f(viewportResLoc, viewportWidth, viewportHeight);
	glBindTexture(GL_TEXTURE_2D, renderTexture);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// UI
	nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

	SDL_GL_SwapWindow(window);
}

// Uninitialize GL
void uninitGL() {
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
}
