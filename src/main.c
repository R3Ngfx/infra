// Infra: GLSL output exporter
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "ui.c"
#include "gl.c"

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
		if (saveVideo) {
			currentFrameTime = (double)currentVideoFrame/frameRate;
			time = currentFrameTime;
			deltaTime = 1.0f/frameRate;
		} else {
			prevFrameTime = currentFrameTime;
			currentFrameTime = SDL_GetTicks()/1000.0;
			deltaTime = currentFrameTime-prevFrameTime;
			if (playing) {
				time += deltaTime;
			}
			if (prevTimeSelect != currentTimeSelect) {
				time = currentTimeSelect/100.0f * renderVideoLength;
				prevTimeSelect = currentTimeSelect;
			} else {
				currentTimeSelect = 100*time/renderVideoLength;
				currentTimeSelect = currentTimeSelect > 100 ? 100 : currentTimeSelect;
				prevTimeSelect = currentTimeSelect;
			}
		}

		// Handle events
		SDL_Event event;
		nk_input_begin(ctx);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT: {
					goto exit;
					break;
				}
				case SDL_KEYDOWN: {
					switch (event.key.keysym.sym) {
						case SDLK_SPACE: {
							//saveFrames = 1;
							break;
						}
					}
					break;
				}
			}
			nk_sdl_handle_event(&event);
		}
		nk_input_end(ctx);

		// Adjust render resoltion
		if (newRenderWidth != renderWidth || newRenderHeight != renderHeight) {
			renderWidth = newRenderWidth;
			renderHeight = newRenderHeight;
			uninitUI();
			uninitGL();
			initGL();
			initUI();
		}

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
