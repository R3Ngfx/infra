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
	if (nk_begin(ctx, "RENDER", nk_rect(viewportWidth-300-gap, gap, 300, 500),
		NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_NO_SCROLLBAR)) {

		/*
		if (nk_tree_push(ctx, NK_TREE_TAB, "INFO", NK_MAXIMIZED)) {
			nk_layout_row_dynamic(ctx, 10, 1);
			nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Viewport Resolution: %ix%i", viewportWidth, viewportHeight);

			nk_layout_row_dynamic(ctx, 10, 1);
			nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "FPS: %2.0f", 1.0/deltaTime);

			nk_tree_pop(ctx);
		}
		*/

		if (nk_tree_push(ctx, NK_TREE_TAB, "CONFIG", NK_MAXIMIZED)) {

			nk_layout_row_dynamic(ctx, 10, 1);
			nk_label(ctx, "Render Resolution:", NK_TEXT_ALIGN_LEFT);
			nk_layout_row_dynamic(ctx, 30, 2);
			nk_property_int(ctx, "Width:", 1, &newRenderWidth, 2147483647, 0, 0);
			nk_property_int(ctx, "Height:", 1, &newRenderHeight, 2147483647, 0, 0);

			nk_layout_row_dynamic(ctx, 10, 2);
			nk_label(ctx, "Render Framerate:", NK_TEXT_ALIGN_LEFT);
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_property_int(ctx, "FPS:", 1, &frameRate, 2147483647, 1, 1);

			nk_layout_row_dynamic(ctx, 10, 2);
			nk_label(ctx, "Render Length:", NK_TEXT_ALIGN_LEFT);
			nk_layout_row_dynamic(ctx, 30, 1);
			nk_property_int(ctx, "Seconds:", 1, &renderVideoLength, 2147483647, 1, 1);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "TIME", NK_MAXIMIZED)) {
			nk_layout_row_begin(ctx, NK_STATIC, 20, 3);
			nk_layout_row_push(ctx, 20);
			if (nk_button_symbol(ctx, playing ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_TRIANGLE_RIGHT)) {
				playing = !playing;
			}
			nk_layout_row_push(ctx, 100);
			nk_labelf(ctx, NK_TEXT_LEFT, "%.2f", time);
			nk_layout_row_end(ctx);
			nk_layout_row_dynamic(ctx, 20, 1);
			nk_progress(ctx, &currentTimeSelect, 100, NK_MODIFIABLE);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "EXPORT", NK_MAXIMIZED)) {
			nk_layout_row_dynamic(ctx, 30, 1);
			if (nk_button_label(ctx, "Save Frame")) {
				saveFrame = 1;
			}

			nk_layout_row_dynamic(ctx, 30, 1);
			if (nk_button_label(ctx, "Save Video")) {
				startVideo = 1;
			}

			if (saveVideo) {
				nk_layout_row_dynamic(ctx, 20, 1);
				nk_label_colored(ctx, "[RECORDING]", NK_TEXT_CENTERED, nk_rgb(255,0,0));
			}
			nk_tree_pop(ctx);
		}
	}
	nk_end(ctx);
}
