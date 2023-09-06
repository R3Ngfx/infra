// Infra: GLSL output exporter
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "ui.c"
#include "render.c"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_keycode.h>
#include <libavcodec/avcodec.h>

int main() {

	// Initialization
	if (!initGL()) {
		error("Error initializing GL\n");
		return 1;
	}
	if (!initUI()) {
		error("Error initializing UI\n");
		return 1;
	}
	initAudio();

	// Main loop
	while (1) {

		// Stop on fatal error
		if (errorCount > 0) {
			if (errorCount > 1) goto exit;
			SDL_Event event;
			nk_input_begin(ctx);
			while (SDL_PollEvent(&event)) {
				nk_sdl_handle_event(&event);
			}
			nk_input_end(ctx);
			renderUI();
			renderGL();
			continue;
		}

		// Frame time
		if (saveVideo) {
			currentFrameTime = (double)currentVideoFrame/frameRate;
			currentTime = currentFrameTime;
			deltaTime = 1.0f/frameRate;
			currentTimeSelect = 100*currentTime/renderVideoLength;
		} else {
			prevFrameTime = currentFrameTime;
			currentFrameTime = SDL_GetTicks()/1000.0;
			deltaTime = currentFrameTime-prevFrameTime;
			if (playing) {
				currentTime += deltaTime;
			}
			if (prevTimeSelect != currentTimeSelect) {
				currentTime = currentTimeSelect/100.0f * renderVideoLength;
				prevTimeSelect = currentTimeSelect;
				seekAudio();
				renderAudio(1);
			} else {
				currentTimeSelect = 100*currentTime/renderVideoLength;
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
							if(!saveVideo) {
								playing = !playing;
								playPauseAudio();
							}
							break;
						}
						case SDLK_LEFT: {
							if (!saveVideo) {
								currentTime -= 1.0/frameRate;
								seekAudio();
								if (currentTime < 0) currentTime = 0;
							}
							break;
						}
						case SDLK_RIGHT: {
							if (!saveVideo) {
								currentTime += 1.0/frameRate;
								seekAudio();
							}
							break;
						}
						case SDLK_F5: {
							reloadShaders = 1;
							break;
						}
						case SDLK_F11: {
							hideUI = !hideUI;
							break;
						}
						case SDLK_F12: {
							saveFrame = 1;
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
			if (!initVideoExport()) {
				error("Error initializing libav\n");
				return 1;
			}
			startVideo = 0;
			saveVideo = 1;
			currentVideoFrame = 0;
			maxVideoFrames = renderVideoLength*frameRate;
			SDL_GL_SetSwapInterval(0);
		}
		if (reloadShaders) {
			setShaders();
			reloadShaders = 0;
		}
		if (reloadTrack) {
			loadTrack(trackPath);
			reloadTrack = 0;
		}
		if (changePauseAudio) {
			changePauseAudio = 0;
			playPauseAudio();
		}

		// Adjust render resoltion
		if (newRenderWidth != renderWidth || newRenderHeight != renderHeight) {
			renderWidth = newRenderWidth;
			renderHeight = newRenderHeight;
			setFBO();
			lastRenderedTime = -1;
		}

		// Rendering
		renderAudio();
		renderUI();
		renderGL();

	}

exit:
	uninitAudio();
	uninitUI();
	uninitGL();
	SDL_Quit();
	return 0;
}
