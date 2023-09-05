// UI functions and handling
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"
#include "string.h"
#include "tinydir/tinydir.h"

#define gap 20

int currentTab = 0;
char* names[] = {"INFO", "SHADER", "AUDIO", "RENDER"};

// Loads selected track
void loadSelectedTrack(char* path) {
	sprintf(trackPath, "%s", path);
	trackPathLen = strlen(shaderPath);
	reloadTrack = 1;
}

// Loads selected shader
void loadSelectedShader(char* path) {
	sprintf(shaderPath, "%s", path);
	shaderPathLen = strlen(shaderPath);
	reloadShaders = 1;

}

// Write directory list for shaders and audio tracks
void writeDirectoryList(tinydir_file file, void (fileFunction)(char*)) {
	if (file.is_dir) {
		if (nk_tree_push(ctx, NK_TREE_TAB, file.name, NK_MAXIMIZED)) {
			tinydir_dir subDir;
			tinydir_open_sorted(&subDir, file.path);
			for (int i = 0; i < subDir.n_files; i++) {
				tinydir_file subFile;
				tinydir_readfile_n(&subDir, &subFile, i);
				if (subFile.name[0] == '.') continue;
				writeDirectoryList(subFile, fileFunction);
			}
			tinydir_close(&subDir);
			nk_tree_pop(ctx);
		}
	} else {
		float ratio[2] = {0.7, 0.3};
		nk_layout_row(ctx, NK_DYNAMIC, 20, 2, ratio);
		nk_label(ctx, file.name, NK_TEXT_ALIGN_LEFT);
		if (nk_button_label(ctx, "Load")) {
			fileFunction(file.path);
		}
	}

}

// Nuklear initialization
int initUI(){
	ctx = nk_sdl_init(window);
	struct nk_font_atlas *atlas;
	nk_sdl_font_stash_begin(&atlas);
	nk_sdl_font_stash_end();
	tinydir_open_sorted(&dataDir, "data");
	return 1;
}

// Nuklear uninitialization
void uninitUI() {
	nk_sdl_shutdown();
}

// UI componentes and structure
void renderUI() {

	if (hideUI) return;

	if (nk_begin(ctx, "CONFIG", nk_rect(viewportWidth-300-gap, gap, 300, 400),
		NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_NO_SCROLLBAR)) {

		nk_layout_row_dynamic(ctx, 20, 4);
		for (int i = 0; i < 4; i++) {
			if (currentTab == i) {
				struct nk_style_item normalButton = ctx->style.button.normal;
				ctx->style.button.normal = ctx->style.button.active;
				nk_button_label(ctx, names[i]);
				ctx->style.button.normal = normalButton;
			} else {
				currentTab = nk_button_label(ctx, names[i]) ? i : currentTab;
			}
		}

		switch (currentTab) {
			case 0:
				nk_layout_row_dynamic(ctx, 200, 1);
				if (nk_group_begin(ctx, "INFO", 0)) {

					nk_layout_row_dynamic(ctx, 10, 1);
					nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Viewport Resolution: %ix%i", viewportWidth, viewportHeight);

					nk_layout_row_dynamic(ctx, 10, 1);
					nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "FPS: %2.0f", 1.0/deltaTime);

					nk_group_end(ctx);
				}
				break;
			case 1:
				nk_layout_row_dynamic(ctx, 400, 1);
				if (nk_group_begin(ctx, "SHADER", 0)) {

					float ratio[] = {0.8, 0.2f};
					nk_layout_row_dynamic(ctx, 10, 1);
					nk_label(ctx, "File Path:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio);
					nk_edit_string(ctx, NK_EDIT_SIMPLE | NK_EDIT_SELECTABLE | NK_EDIT_CLIPBOARD, shaderPath, &shaderPathLen, 256, nk_filter_default);
					if (nk_button_label(ctx, "Load")) {
						// Load shader
						shaderPath[shaderPathLen] = '\0';
						reloadShaders = 1;
					}

					tinydir_file file;
					tinydir_file_open(&file, "data/shaders");
					writeDirectoryList(file, loadSelectedShader);

					nk_group_end(ctx);
				}
				break;
			case 2:
				nk_layout_row_dynamic(ctx, 400, 1);
				if (nk_group_begin(ctx, "AUDIO", 0)) {

					float ratio[] = {0.8, 0.2f};
					nk_layout_row_dynamic(ctx, 10, 1);
					nk_label(ctx, "File Path:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio);
					nk_edit_string(ctx, NK_EDIT_SIMPLE | NK_EDIT_SELECTABLE | NK_EDIT_CLIPBOARD, trackPath, &trackPathLen, 256, nk_filter_default);
					if (nk_button_label(ctx, "Load")) {
						// Load track
						trackPath[trackPathLen] = '\0';
						reloadTrack = 1;
					}

					tinydir_file file;
					tinydir_file_open(&file, "data/audio");
					writeDirectoryList(file, loadSelectedTrack);

					nk_layout_row_dynamic(ctx, 10, 2);
					nk_label(ctx, "Visualization:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_float(ctx, "Smoothness", 0, &smoothness, frameRate, 0.1, 0.1);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_float(ctx, "Power", 0.01, &power, 20, 0.1, 0.1);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_float(ctx, "Drop", 0, &drop, 1, 0.1, 0.1);
					nk_group_end(ctx);
				}
				break;
			case 3:
				nk_layout_row_dynamic(ctx, 300, 1);
				if (nk_group_begin(ctx, "RENDER", 0)) {

					nk_layout_row_dynamic(ctx, 10, 1);
					nk_label(ctx, "Resolution:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 30, 2);
					nk_property_int(ctx, "Width:", 1, &newRenderWidth, 2147483647, 0, 0);
					nk_property_int(ctx, "Height:", 1, &newRenderHeight, 2147483647, 0, 0);

					nk_layout_row_dynamic(ctx, 10, 2);
					nk_label(ctx, "Bitrate:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_int(ctx, "Bitrate(kb/s):", 1, &videoBitrate, 2147483647, 1, 1);

					nk_layout_row_dynamic(ctx, 10, 2);
					nk_label(ctx, "Framerate:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_int(ctx, "FPS:", 1, &frameRate, 2147483647, 1, 1);

					nk_layout_row_dynamic(ctx, 10, 2);
					nk_label(ctx, "Length:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_float(ctx, "Seconds:", 0, &renderVideoLength, 3600, 1, 0.1);

					nk_group_end(ctx);
				}
				break;
			default:
				break;
		}

	}
	nk_end(ctx);

	if (nk_begin(ctx, "TIMELINE", nk_rect(gap, viewportHeight-gap-65, viewportWidth-2*gap, 65),
		NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
		float ratio[] = {0.8, 0.1, 0.1};
		nk_layout_row(ctx, NK_DYNAMIC, 30, 3, ratio);
		nk_label(ctx, "TIMELINE", NK_TEXT_ALIGN_LEFT);
		if (nk_button_label(ctx, "Save Frame")) saveFrame = 1;
		if (nk_button_label(ctx, "Save Video")) startVideo = 1;
		nk_layout_row_begin(ctx, NK_STATIC, 20, 3);
		nk_layout_row_push(ctx, 20);
		if (nk_button_symbol(ctx, playing ? NK_SYMBOL_RECT_SOLID : NK_SYMBOL_TRIANGLE_RIGHT)) {
			if (!saveVideo){
				playing = !playing;
				changePauseAudio = 1;
			}
		}
		nk_layout_row_push(ctx, 30);
		nk_labelf(ctx, NK_TEXT_LEFT, "%.2f", currentTime);
		nk_layout_row_push(ctx, viewportWidth-2*gap-50-20);
		nk_progress(ctx, &currentTimeSelect, 100, NK_MODIFIABLE);
		nk_layout_row_end(ctx);
	}
	nk_end(ctx);
}
