// UI functions and handling
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"

#define gap 20

// Nuklear initialization
int initUI(){
	// Initialize nuklear
	ctx = nk_sdl_init(window);
	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	nk_sdl_font_stash_end();
	return 1;

}

// Uninitialize UI
void uninitUI() {
	nk_sdl_shutdown();
}

void renderUI() {
	if (nk_begin(ctx, "INFO", nk_rect(viewportWidth-300-gap, gap, 300, 500), NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MINIMIZABLE)) {

		char s[512];
		nk_layout_row_dynamic(ctx, 10, 1);
		sprintf(s, "Viewport resolution: %ix%i", viewportWidth, viewportHeight);
		nk_label(ctx, s, NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 10, 1);
		sprintf(s, "Render resolution: %ix%i", renderWidth, renderHeight);
		nk_label(ctx, s, NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 10, 1);
		sprintf(s, "Framerate: %i", (int)(1/(deltaTime)));
		nk_label(ctx, s, NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 10, 1);
		sprintf(s, "Time: %.2f", currFrameTime);
		nk_label(ctx, s, NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 10, 1);
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Save frame")) {
			saveFrames = 1;
		}

	}
	nk_end(ctx);
}
