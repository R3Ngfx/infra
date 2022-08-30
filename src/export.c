// Image and video exporting
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

void saveRender(char* path) {
	unsigned char renderPixels[3*renderWidth*renderHeight];
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, renderWidth, renderHeight, GL_RGB, GL_UNSIGNED_BYTE, &renderPixels);
	SDL_Surface* saveSurf = SDL_CreateRGBSurfaceFrom(renderPixels, renderWidth, renderHeight, 24, 3*renderWidth, 0, 0, 0, 0);
	SDL_SaveBMP(saveSurf, path);
}

void exportFrame() {
	saveRender("out/frame.bmp");
	printf("Saved Frame\n");
	saveFrame = 0;
}

void exportVideo() {
	char s[512];
	sprintf(s, "out/frame%04d.bmp", currentVideoFrame);
	saveRender(s);
	printf("Saved Video: Frame %04d\n", currentVideoFrame);
	currentVideoFrame++;
	if (currentVideoFrame >= maxVideoFrames) saveVideo = 0;
}
