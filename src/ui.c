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

// UI componentes and structure
void renderUI() {
	if (nk_begin(ctx, "OUTPUT", nk_rect(viewportWidth-300-gap, gap, 300, 500),
		NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_NO_SCROLLBAR)) {

		char s[512];
		nk_layout_row_dynamic(ctx, 10, 1);
		sprintf(s, "Viewport resolution: %ix%i", viewportWidth, viewportHeight);
		nk_label(ctx, s, NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 10, 1);
		nk_label(ctx, "Render resolution:", NK_TEXT_ALIGN_LEFT);
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_property_int(ctx, "Width:", 1, &newRenderWidth, 2147483647, 0, 0);
		nk_property_int(ctx, "Height:", 1, &newRenderHeight, 2147483647, 0, 0);

		nk_layout_row_dynamic(ctx, 10, 2);
		nk_label(ctx, "Render Framerate:", NK_TEXT_ALIGN_LEFT);
		nk_layout_row_dynamic(ctx, 30, 1);
		nk_property_int(ctx, "FPS:", 1, &frameRate, 2147483647, 1, 1);

		nk_layout_row_dynamic(ctx, 20, 1);
		sprintf(s, "Time: %.2f", currFrameTime);
		nk_label(ctx, s, NK_TEXT_ALIGN_LEFT);

		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Save frame")) {
			saveFrame = 1;
		}

	}
	nk_end(ctx);
}
