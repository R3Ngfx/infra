// UI functions and handling
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"
#include "string.h"
#include "tinydir/tinydir.h"

#define gap 20

int currentTab = 0;
int directoryListID = 0;
char* names[] = {"INFO", "SHADER", "TEXTURE", "AUDIO", "RENDER"};

// Loads selected track
void loadSelectedTrack(char* path) {
	sprintf(trackPath, "%s", path);
	trackPathLen = strlen(trackPath);
	reloadTrack = 1;
}

// Loads selected shader
void loadSelectedShader(char* path) {
	sprintf(shaderPath, "%s", path);
	shaderPathLen = strlen(shaderPath);
	reloadShaders = 1;
}

// Loads selected texture
void loadSelectedTexture(char* path) {
	if (loadedTextures >= 16) return;
	sprintf(texturePaths[loadedTextures], "%s", path);
	sprintf(texturePath, "%s", path);
	texturePathLen = strlen(texturePath);
	loadedTextures++;
	reloadTexture = 1;
}

// Write file load directory list recursively
void writeDirectoryListRecursive(tinydir_file file, void (fileFunction)(char*)) {
	if (file.is_dir) {
		if (nk_tree_push_id(ctx, NK_TREE_TAB, file.name, directoryListID ? NK_MINIMIZED : NK_MAXIMIZED, directoryListID++)) {
			tinydir_dir subDir;
			tinydir_open_sorted(&subDir, file.path);
			for (int i = 0; i < subDir.n_files; i++) {
				tinydir_file subFile;
				tinydir_readfile_n(&subDir, &subFile, i);
				if (subFile.name[0] == '.') continue;
				writeDirectoryListRecursive(subFile, fileFunction);
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

void writeDirectoryList (char* path, void (fileFunction)(char*)) {
	tinydir_file file;
	tinydir_file_open(&file, path);
	directoryListID = 0;
	writeDirectoryListRecursive(file, fileFunction);
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

// UI components and structure
void renderUI() {

	if (errorCount != 0) {
		if (nk_begin(ctx, "ERROR", nk_rect(viewportWidth/2-200, viewportHeight/2-100, 400, 200),
			NK_WINDOW_TITLE|NK_WINDOW_BORDER)) {

			nk_layout_row_dynamic(ctx, 125, 1);
			nk_label_wrap(ctx, errorMsg);
			float ratio[3] = {0.3333, 0.3333, 0.333};
			nk_layout_row(ctx, NK_DYNAMIC, 20, 3, ratio);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
			if (nk_button_label(ctx, "Close")) {
				errorCount = 2;
			}
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		nk_end(ctx);
		return;
	}

	if (hideUI) return;

	if (saveVideo) {
		if (nk_begin(ctx, "RECORDING", nk_rect(viewportWidth/2-200, viewportHeight/2-100, 400, 200),
			NK_WINDOW_TITLE|NK_WINDOW_BORDER)) {

			nk_layout_row_dynamic(ctx, 60, 1);
			nk_layout_row_dynamic(ctx, 60, 1);
			nk_labelf(ctx, NK_TEXT_ALIGN_CENTERED, "RECORDING [%i/%i]", currentVideoFrame, maxVideoFrames);
			float ratio[3] = {0.3333, 0.3333, 0.333};
			nk_layout_row(ctx, NK_DYNAMIC, 20, 3, ratio);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
			if (nk_button_label(ctx, "Stop")) {
				currentVideoFrame = maxVideoFrames;
			}
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		nk_end(ctx);
	}

	if (nk_begin(ctx, "CONFIG", nk_rect(viewportWidth-350-gap, gap, 350, 500),
		NK_WINDOW_BORDER|NK_WINDOW_TITLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_NO_SCROLLBAR)) {

		nk_layout_row_dynamic(ctx, 20, 5);
		for (int i = 0; i < 5; i++) {
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
				nk_layout_row_dynamic(ctx, 400, 1);
				if (nk_group_begin(ctx, "INFO", 0)) {

					if (nk_tree_push(ctx, NK_TREE_TAB, "INFO", NK_MAXIMIZED)) {
						nk_layout_row_dynamic(ctx, 10, 1);
						nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Render Resolution: %ix%i", renderWidth, renderHeight);
						nk_layout_row_dynamic(ctx, 10, 1);
						nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Viewport Resolution: %ix%i", viewportWidth, viewportHeight);
						nk_layout_row_dynamic(ctx, 10, 1);
						nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "FPS: %2.0f", 1.0/deltaTime);
						nk_tree_pop(ctx);
					}

					if (nk_tree_push(ctx, NK_TREE_TAB, "KEY BINDINGS", NK_MAXIMIZED)) {
						nk_layout_row_dynamic(ctx, 10, 1);
						nk_label(ctx, "[SPACE]         Time play/stop", NK_TEXT_ALIGN_LEFT);
						nk_layout_row_dynamic(ctx, 10, 1);
						nk_label(ctx, "[LEFT ARROW]    Time previous frame", NK_TEXT_ALIGN_LEFT);
						nk_layout_row_dynamic(ctx, 10, 1);
						nk_label(ctx, "[RIGHT ARROW]   Time next frame", NK_TEXT_ALIGN_LEFT);
						nk_layout_row_dynamic(ctx, 10, 1);
						nk_label(ctx, "[F5]            Reload shaders", NK_TEXT_ALIGN_LEFT);
						nk_layout_row_dynamic(ctx, 10, 1);
						nk_label(ctx, "[F11]           UI hide/show", NK_TEXT_ALIGN_LEFT);
						nk_tree_pop(ctx);
					}

					if (nk_tree_push(ctx, NK_TREE_TAB, "HOW TO USE", NK_MAXIMIZED)) {
						if (nk_tree_push(ctx, NK_TREE_NODE, "LOAD SHADERS", NK_MINIMIZED)) {
							nk_layout_row_dynamic(ctx, 40, 1);
							nk_label_wrap(ctx, "You can load the rendered shader in the [SHADER] tab.");
							nk_layout_row_dynamic(ctx, 40, 1);
							nk_label_wrap(ctx, "A list is shown of all of the files present in the data directory.");
							nk_layout_row_dynamic(ctx, 50, 1);
							nk_label_wrap(ctx, "In order to load any file in any other location you can specify the local or global filepath.");
							nk_tree_pop(ctx);
						}
						if (nk_tree_push(ctx, NK_TREE_NODE, "USING TEXTURES", NK_MINIMIZED)) {
							nk_layout_row_dynamic(ctx, 40, 1);
							nk_label_wrap(ctx, "Up to 16 textures can be loaded in the [TEXTURE] tab.");
							nk_layout_row_dynamic(ctx, 50, 1);
							nk_label_wrap(ctx, "The textures will be passed to the shader as Sampler2Ds with names from tex0 to tex15.");
							nk_layout_row_dynamic(ctx, 40, 1);
							nk_label_wrap(ctx, "You can change the order of the loaded textures as well as unloading them.");
							nk_tree_pop(ctx);
						}
						if (nk_tree_push(ctx, NK_TREE_NODE, "AUDIO", NK_MINIMIZED)) {
							nk_layout_row_dynamic(ctx, 50, 1);
							nk_label_wrap(ctx, "You can add an audio track that will be included in the rendered video sequence.");
							nk_layout_row_dynamic(ctx, 40, 1);
							nk_label_wrap(ctx, "An audio file can be loaded in the [AUDIO] tab.");
							nk_layout_row_dynamic(ctx, 50, 1);
							nk_label_wrap(ctx, "You can perform audio visualization effects in the shaders by reading the audio vec4 uniform.");
							nk_layout_row_dynamic(ctx, 50, 1);
							nk_label_wrap(ctx, "The behavior of the audio detection can be configured by modifying the smoothness, power and drop variables.");
							nk_tree_pop(ctx);
						}
						if (nk_tree_push(ctx, NK_TREE_NODE, "EXPORTING", NK_MINIMIZED)) {
							nk_layout_row_dynamic(ctx, 40, 1);
							nk_label_wrap(ctx, "You can configure the rendering setings in the [RENDER] tab.");
							nk_layout_row_dynamic(ctx, 50, 1);
							nk_label_wrap(ctx, "A video sequence can be exported by pressing [Save Video] in the timeline window below.");
							nk_layout_row_dynamic(ctx, 40, 1);
							nk_label_wrap(ctx, "A single image of the current frame can be exported by pressing [Save Frame].");
							nk_tree_pop(ctx);
						}
						nk_tree_pop(ctx);
					}

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

					writeDirectoryList("data/shaders", loadSelectedShader);

					nk_group_end(ctx);
				}
				break;
			case 2:
				nk_layout_row_dynamic(ctx, 400, 1);
				if (nk_group_begin(ctx, "TEXTURE", 0)) {

					nk_layout_row_dynamic(ctx, 20, 1);
					nk_label(ctx, "LOADED:", NK_TEXT_LEFT);
					for (int i = 0; i < loadedTextures; i++) {
						float ratio[5] = {0.125, 0.65, 0.075, 0.075, 0.075};
						nk_layout_row(ctx, NK_DYNAMIC, 20, 5, ratio);
						char texName[8];
						sprintf(texName, "tex%i: ", i);
						nk_label(ctx, texName, NK_TEXT_ALIGN_LEFT);
						nk_label(ctx, texturePaths[i], NK_TEXT_ALIGN_RIGHT);
						if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_UP)) {
							if (i == 0) break;
							char tempName[4096];
							sprintf(tempName, "%s", texturePaths[i-1]);
							sprintf(texturePaths[i-1], "%s", texturePaths[i]);
							sprintf(texturePaths[i], "%s", tempName);
							reloadTexture = 1;
						}
						if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_DOWN)) {
							if (i >= loadedTextures-1) break;
							char tempName[4096];
							sprintf(tempName, "%s", texturePaths[i+1]);
							sprintf(texturePaths[i+1], "%s", texturePaths[i]);
							sprintf(texturePaths[i], "%s", tempName);
							reloadTexture = 1;
						}
						if (nk_button_symbol(ctx, NK_SYMBOL_X)) {
							loadedTextures--;
							for (int i = 0; i < loadedTextures; i++) {
								sprintf(texturePaths[i], "%s", texturePaths[i+1]);
							}
							reloadTexture = 1;
						}
					}

					nk_layout_row_dynamic(ctx, 10, 1);
					nk_layout_row_dynamic(ctx, 20, 1);
					nk_label(ctx, "LOAD:", NK_TEXT_LEFT);

					float ratio[] = {0.8, 0.2f};
					nk_layout_row_dynamic(ctx, 10, 1);
					nk_label(ctx, "File Path:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratio);
					nk_edit_string(ctx, NK_EDIT_SIMPLE | NK_EDIT_SELECTABLE | NK_EDIT_CLIPBOARD, texturePath, &texturePathLen, 256, nk_filter_default);
					if (nk_button_label(ctx, "Load")) {
						// Load texture
						texturePath[texturePathLen] ='\0';
						loadSelectedTexture(texturePath);
					}

					writeDirectoryList("data/textures", loadSelectedTexture);

					nk_group_end(ctx);
				}
				break;
			case 3:
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

					writeDirectoryList("data/audio", loadSelectedTrack);

					nk_layout_row_dynamic(ctx, 10, 0);
					nk_layout_row_dynamic(ctx, 20, 2);
					nk_label(ctx, "VISUALIZATION:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_float(ctx, "Smoothness", 0, &smoothness, 1, 0.1, 0.1);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_float(ctx, "Power", 0.01, &power, 20, 0.1, 0.1);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_property_float(ctx, "Drop", 0, &drop, 1, 0.1, 0.1);
					nk_group_end(ctx);
				}
				break;
			case 4:
				nk_layout_row_dynamic(ctx, 400, 1);
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

					nk_layout_row_dynamic(ctx, 10, 1);
					nk_label(ctx, "Frame export path:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_edit_string(ctx, NK_EDIT_SIMPLE | NK_EDIT_SELECTABLE | NK_EDIT_CLIPBOARD, framePath, &framePathLen, 256, nk_filter_default);
					nk_layout_row_dynamic(ctx, 10, 1);
					nk_label(ctx, "Video export path:", NK_TEXT_ALIGN_LEFT);
					nk_layout_row_dynamic(ctx, 30, 1);
					nk_edit_string(ctx, NK_EDIT_SIMPLE | NK_EDIT_SELECTABLE | NK_EDIT_CLIPBOARD, videoPath, &videoPathLen, 256, nk_filter_default);
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
		if (nk_button_label(ctx, "Save Frame")) {
			framePath[framePathLen] = '\0';
			saveFrame = 1;
		}
		if (nk_button_label(ctx, "Save Video")) {
			videoPath[videoPathLen] = '\0';
			startVideo = 1;
		}
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
		nk_progress(ctx, &currentTimeSelected, 100, NK_MODIFIABLE);
		if (currentTimeSelected != lastTimeSelected && !saveVideo && currentTimeSelected != 100){
			currentTime = renderVideoLength*currentTimeSelected/100.0f;
			seekedTime = 1;
		}
		nk_layout_row_end(ctx);
	}
	nk_end(ctx);

	if (warningCount > 0) {
		if (nk_begin(ctx, "WARNING", nk_rect(viewportWidth/2-200, viewportHeight/2-100, 400, 200),
			NK_WINDOW_TITLE|NK_WINDOW_BORDER)) {

			nk_layout_row_dynamic(ctx, 125, 1);
			nk_label_wrap(ctx, warningMsg[warningCount-1]);
			float ratio[3] = {0.3333, 0.3333, 0.333};
			nk_layout_row(ctx, NK_DYNAMIC, 20, 3, ratio);
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
			if (nk_button_label(ctx, "Close")) {
				warningCount--;
			}
			nk_label(ctx, "", NK_TEXT_ALIGN_LEFT);
		}
		nk_end(ctx);
	}
}
