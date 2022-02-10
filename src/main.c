// Infra: GLSL output exporter
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "ui.c"
#include "gl.c"

#include <fftw3.h>

int main() {

	// Load GL
	if (!initGL()) {
		printf("Error initializing GL\n");
		return 1;
	}

	// Load UI
	if (!initUI(window)) {
		printf("Error initializing UI\n");
		return 1;
	}

	// Main loop
	while (1) {

		// Frame time
		prevFrameTime = currFrameTime;
		currFrameTime = SDL_GetTicks()/1000.0;
		deltaTime = currFrameTime-prevFrameTime;

		// Handle events
		SDL_Event event;
		nk_input_begin(ctx);
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) goto exit;
			nk_sdl_handle_event(&event);
		}
		nk_input_end(ctx);

		// Render UI
		renderUI();

		// Render GL
		renderGL();

	}

exit:
	uninitUI();
	uninitGL();
	SDL_Quit();
	return 0;
}
