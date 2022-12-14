// Audio functions and handling
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"
#include "miniaudio/miniaudio.h"
#include <SDL2/SDL_audio.h>

SDL_AudioSpec spec;
unsigned int trackLength = 0;
unsigned char* trackBuffer;

void loadTrack(char* path) {
	SDL_LoadWAV(path, &spec, &trackBuffer, &trackLength);
	SDL_FreeWAV(trackBuffer);
}
