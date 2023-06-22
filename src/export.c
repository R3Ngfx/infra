// Image and video exporting
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
#include <stdio.h>
#include <sys/types.h>

const AVCodec* codec;
struct AVCodecContext* codecContext = NULL;
AVFrame* frame;
struct AVPacket* packet;
const u_int8_t endcode[] = {0,0,1,0xb7};
FILE* videoFile;
struct SwsContext* swsContext = NULL;
unsigned char* renderPixels;

int initExport() {
	// Initialize encoder
	codec = avcodec_find_encoder(AV_CODEC_ID_MPEG2VIDEO);
	if (!codec) {
		printf("Codec mpeg2video not found\n");
		return 0;
	}
	codecContext = avcodec_alloc_context3(codec);
	if (!codecContext) {
		printf("Error allocatinc codec context\n");
		return 0;
	}
	codecContext->bit_rate = 1000*bitrate;
	codecContext->width = renderWidth;
	codecContext->height = renderHeight;
	codecContext->time_base = (AVRational){1, frameRate};
	codecContext->framerate = (AVRational){frameRate, 1};
	codecContext->gop_size = 15;
	codecContext->max_b_frames = 1;
	codecContext->pix_fmt = AV_PIX_FMT_YUV422P;
	if (avcodec_open2(codecContext, codec, NULL) < 0) {
		printf("Error opening codec\n");
		return 0;
	}
	packet = av_packet_alloc();
	if (!packet) {
		printf("Error allocating package\n");
		return 0;
	}
	videoFile = fopen("out/video.mp4", "wb");
	if (!videoFile) {
		printf("Error opening video file\n");
		return 0;
	}
	frame = av_frame_alloc();
	if (!frame) {
		printf("Error allocating video frame\n");
		return 0;
	}
	frame->format = codecContext->pix_fmt;
	frame->width = codecContext->width;
	frame->height = codecContext->height;
	if (av_frame_get_buffer(frame, 0) < 0) {
		printf("Error allocating video frame data\n");
		return 0;
	}
	swsContext = sws_getContext(renderWidth, renderHeight, AV_PIX_FMT_RGB24, renderWidth, renderHeight, AV_PIX_FMT_YUV422P, SWS_SPLINE, NULL, NULL, NULL);
	renderPixels = malloc(3*renderWidth*renderHeight);
	return 1;
}

void uninitExport() {
	if(renderPixels != NULL) free(renderPixels);
}

void exportFrame() {
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, renderWidth, renderHeight, GL_BGR, GL_UNSIGNED_BYTE, renderPixels);
	SDL_Surface* saveSurf = SDL_CreateRGBSurfaceFrom(renderPixels, renderWidth, renderHeight, 24, 3*renderWidth, 0, 0, 0, 0);
	SDL_SaveBMP(saveSurf, "out/frame.bmp");
	SDL_FreeSurface(saveSurf);
	printf("Saved Frame\n");
	saveFrame = 0;
}

void encode(int final) {
	if (avcodec_send_frame(codecContext, final ? NULL : frame) < 0) {
		printf("Error sending a frame for encoding\n");
	}
	int ret = 0;
	while (ret >= 0) {
		ret = avcodec_receive_packet(codecContext, packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return;
		} else if (ret < 0) {
			printf("Error during encoding\n");
			return;
		}
		fwrite(packet->data, 1, packet->size, videoFile);
		av_packet_unref(packet);
	}
}

void encodeVideoFrame() {
	if (av_frame_make_writable(frame) < 0) {
		printf("Frame data is not writeable\n");
		return;
	}
	int rgbStride[1] = { 3*renderWidth };
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, renderWidth, renderHeight, GL_RGB, GL_UNSIGNED_BYTE, renderPixels);
	sws_scale(swsContext, &renderPixels, rgbStride, 0, renderHeight, frame->data, frame->linesize);
	frame->pts = currentVideoFrame;
	encode(0);
	currentVideoFrame++;
	if (currentVideoFrame >= maxVideoFrames) {
		encode(1);
		fwrite(endcode, 1, sizeof(endcode), videoFile);
		fclose(videoFile);
		avcodec_free_context(&codecContext);
		av_frame_free(&frame);
		av_packet_free(&packet);
		saveVideo = 0;
		SDL_GL_SetSwapInterval(1);
		printf("Saved Video\n");
	}
}
