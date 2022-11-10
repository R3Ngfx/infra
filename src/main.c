// Infra: GLSL output exporter
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "ui.c"
#include "render.c"
#include <SDL2/SDL_keycode.h>
#include <libavcodec/avcodec.h>

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
			currentTimeSelect = 100*time/renderVideoLength;
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

		// Handle input events
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
							playing = !playing;
							break;
						}
						case SDLK_LEFT: {
							if (!saveVideo) {
								time -= 1.0/frameRate;
								if (time < 0) time = 0;
							}
							break;
						}
						case SDLK_RIGHT: {
							if (!saveVideo) {
								time += 1.0/frameRate;
							}
							break;
						}
						case SDLK_F5: {
							setShaders();
							break;
						}
					}
					break;
				}
			}
			nk_sdl_handle_event(&event);
		}
		nk_input_end(ctx);

		// Handle program events
		if (startVideo) {
			if (!initExport()) {
				printf("Error initializing libav\n");
				return 1;
			}
			startVideo = 0;
			saveVideo = 1;
			currentVideoFrame = 0;
			maxVideoFrames = renderVideoLength*frameRate;
		}

		// Adjust render resoltion
		if (newRenderWidth != renderWidth || newRenderHeight != renderHeight) {
			renderWidth = newRenderWidth;
			renderHeight = newRenderHeight;
			setFBO();
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
