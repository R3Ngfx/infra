// Image and video exporting
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#include "global.c"
#include "audio.c"
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <stdio.h>
#include <sys/types.h>

// Wrapper around output AVStream
struct OutputStream {
	AVStream* stream;
	AVCodecContext* codecContext;
	AVPacket* packet;
	int samples;
	AVFrame* frame;
	AVFrame* tempFrame;
};

struct OutputStream videoStream, audioStream;
const AVCodec *videoCodec, *audioCodec;
AVFormatContext *outputContext;
const AVOutputFormat* format;
unsigned char* renderPixels;
struct SwsContext* swsContext;
struct SwrContext* swrContext;

// Sets video encoder depending on the codec format
int initVideoStream() {
	// Allocate context
	videoCodec = avcodec_find_encoder(format->video_codec);
	if (!(videoCodec)) {
		printf("Error finding video encoder\n");
		return 0;
	}
	videoStream.packet = av_packet_alloc();
	if (!videoStream.packet) {
		printf("Error allocating video packet\n");
		return 0;
	}
	videoStream.stream = avformat_new_stream(outputContext, NULL);
	if (!videoStream.stream) {
		printf("Error allocating video stream\n");
		return 0;
	}
	videoStream.stream->id = outputContext->nb_streams-1;
	videoStream.codecContext = avcodec_alloc_context3(videoCodec);
	if (!videoStream.codecContext) {
		printf("Error allocating video codec context\n");
		return 0;
	}
	// Assign video format
	videoStream.codecContext->codec_id = format->video_codec;
	videoStream.codecContext->bit_rate = 1000*videoBitrate;
	videoStream.codecContext->width = renderWidth;
	videoStream.codecContext->height = renderHeight;
	videoStream.codecContext->time_base = (AVRational){1, frameRate};
	videoStream.stream->time_base = (AVRational){1, frameRate};
	videoStream.codecContext->gop_size = 15;
	videoStream.codecContext->max_b_frames = 1;
	videoStream.codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	// Separate global header for specific formats
	if (outputContext->oformat->flags & AVFMT_GLOBALHEADER) {
		videoStream.codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	// Open codec
	if (avcodec_open2(videoStream.codecContext, videoCodec, NULL) < 0) {
		printf("Error opening video codec\n");
		return 0;
	}
	// Init reusable frame for writing
	videoStream.frame = av_frame_alloc();
	videoStream.frame->format = videoStream.codecContext->pix_fmt;
	videoStream.frame->width = videoStream.codecContext->width;
	videoStream.frame->height = videoStream.codecContext->height;
	if (av_frame_get_buffer(videoStream.frame, 0) < 0) {
		printf("Error allocating video frame\n");
		return 0;
	}
	// Copy stream parameters to the muxer
	if (avcodec_parameters_from_context(videoStream.stream->codecpar, videoStream.codecContext) < 0) {
		printf("Error copying stream parameters\n");
		return 0;
	}
	swsContext = sws_getContext(renderWidth, renderHeight, AV_PIX_FMT_RGB24, renderWidth, renderHeight,
		AV_PIX_FMT_YUV420P, SWS_SPLINE, NULL, NULL, NULL);
	return 1;
}

// Sets audio encoder depending on the codec format
int initAudioStream() {
	// Allocate context
	audioCodec = avcodec_find_encoder(format->audio_codec);
	if (!(audioCodec)) {
		printf("Error finding audio encoder\n");
		return 0;
	}
	audioStream.packet = av_packet_alloc();
	if (!audioStream.packet) {
		printf("Error allocating audio packet\n");
		return 0;
	}
	audioStream.stream = avformat_new_stream(outputContext, NULL);
	if (!audioStream.stream) {
		printf("Error allocating audio stream\n");
		return 0;
	}
	audioStream.stream->id = outputContext->nb_streams-1;
	audioStream.codecContext = avcodec_alloc_context3(audioCodec);
	if (!audioStream.codecContext) {
		printf("Error allocating audio codec context\n");
		return 0;
	}
	// Assign audio format
	audioStream.codecContext->sample_fmt = audioCodec->sample_fmts ? audioCodec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
	audioStream.codecContext->bit_rate = 1000*audioBitrate;
	audioStream.codecContext->sample_rate = outputSampleRate;
	AVChannelLayout cl = ((trackChannels == 2) ? (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO : (AVChannelLayout)AV_CHANNEL_LAYOUT_MONO);
	av_channel_layout_copy(&audioStream.codecContext->ch_layout, &cl);
	audioStream.stream->time_base = (AVRational){1, outputSampleRate};
	// Separate global header for specific formats
	if (outputContext->oformat->flags & AVFMT_GLOBALHEADER) {
		audioStream.codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}
	// Open codec
	if (avcodec_open2(audioStream.codecContext, audioCodec, NULL) < 0) {
		printf("Error opening audio codec\n");
		return 0;
	}
	// Init frames
	audioStream.frame = av_frame_alloc();
	if (!audioStream.frame) {
		printf("Error allocationg audio frame\n");
		return 0;
	}
	audioStream.frame->format = audioStream.codecContext->sample_fmt;
	av_channel_layout_copy(&audioStream.frame->ch_layout, &audioStream.codecContext->ch_layout);
	audioStream.frame->sample_rate = audioStream.codecContext->sample_rate;
	audioStream.frame->nb_samples = audioStream.codecContext->frame_size;
	if (av_frame_get_buffer(audioStream.frame, 0) < 0) {
		printf("Error allocating audio buffer\n");
		return 0;
	}
	audioStream.tempFrame = av_frame_alloc();
	if (!audioStream.tempFrame) {
		printf("Error allocationg audio temporary frame\n");
		return 0;
	}
	audioStream.tempFrame->format = audioStream.codecContext->sample_fmt;
	av_channel_layout_copy(&audioStream.tempFrame->ch_layout, &audioStream.codecContext->ch_layout);
	audioStream.tempFrame->sample_rate = audioStream.codecContext->sample_rate;
	audioStream.tempFrame->nb_samples = audioStream.codecContext->frame_size;
	audioStream.tempFrame->pts = -audioStream.tempFrame->nb_samples;
	if (av_frame_get_buffer(audioStream.tempFrame, 0) < 0) {
		printf("Error allocating audio temporary buffer\n");
		return 0;
	}
	// Copy parameters to muxer
	if (avcodec_parameters_from_context(audioStream.stream->codecpar, audioStream.codecContext)) {
		printf("Error copying stream parameters\n");
		return 0;
	}
	// Init resampler
	swrContext = swr_alloc();
	if (!swrContext) {
		printf("Error allocating audio resampler\n");
		return 0;
	}
	av_opt_set_chlayout(swrContext, "in_chlayout", &audioStream.codecContext->ch_layout, 0);
	av_opt_set_int(swrContext, "in_sample_rate", trackSampleRate, 0);
	av_opt_set_sample_fmt(swrContext, "in_sample_fmt", AV_SAMPLE_FMT_U8, 0);
	av_opt_set_chlayout(swrContext, "out_chlayout", &audioStream.codecContext->ch_layout, 0);
	av_opt_set_int(swrContext, "out_sample_rate", audioStream.codecContext->sample_rate, 0);
	av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
	if (swr_init(swrContext) < 0) {
		printf("Error initializing audio resampler\n");
		return 0;
	}
	return 1;
}

int initVideoExport() {
	// Allocate media context
	avformat_alloc_output_context2(&outputContext, NULL, NULL, videoFilename);
	if (!outputContext) {
		printf("Error allocating output context\n");
		return 0;
	}
	format = outputContext->oformat;
	// Add audio and video streams and initialize codecs
	if (format->video_codec != AV_CODEC_ID_NONE) {
		if (!initVideoStream()) return 0;
		if (!initAudioStream()) return 0;
	}
	// Write format and header to file
	av_dump_format(outputContext, 0, videoFilename, AVIO_FLAG_WRITE);
	if (!(format->flags & AVFMT_NOFILE)) {
		if (avio_open(&outputContext->pb, videoFilename, AVIO_FLAG_WRITE) < 0) {
			printf("Error opening %s\n", videoFilename);
			return 0;
		}
	}
	if (avformat_write_header(outputContext, NULL) < 0) {
		printf("Error writing header\n");
		return 0;
	}
	return 1;
}

void uninitVideoExport() {
	avcodec_free_context(&videoStream.codecContext);
	av_frame_free(&videoStream.frame);
	av_packet_free(&videoStream.packet);
	avcodec_free_context(&audioStream.codecContext);
	av_frame_free(&audioStream.frame);
	av_frame_free(&audioStream.tempFrame);
	av_packet_free(&audioStream.packet);
	sws_freeContext(swsContext);
	swr_free(&swrContext);
	if (!(format->flags & AVFMT_NOFILE)) {
		avio_closep(&outputContext->pb);
	}
	avformat_free_context(outputContext);
}

// Encodes frame into stream (assumes frame has already been assigned)
int encode(struct OutputStream outputStream) {
	int ret = 0;
	if ((ret = avcodec_send_frame(outputStream.codecContext, outputStream.frame)) < 0) {
		printf("Error sending frame for encoding (%s)\n", av_err2str(ret));
		return 0;
	}
	while (ret >= 0) {
		ret = avcodec_receive_packet(outputStream.codecContext, outputStream.packet);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return 1;
		} else if (ret < 0) {
			printf("Error during encoding (%s)\n", av_err2str(ret));
			return 0;
		}
		av_packet_rescale_ts(outputStream.packet, outputStream.codecContext->time_base, outputStream.stream->time_base);
		outputStream.packet->stream_index = outputStream.stream->index;
		ret = av_interleaved_write_frame(outputContext, outputStream.packet);
		if (ret < 0) {
			printf("Error while writing output packet (%s)\n", av_err2str(ret));
			return 0;
		}
	}
	return 1;
}

void encodeVideoFrame() {
	if (av_compare_ts(currentVideoFrame, videoStream.codecContext->time_base,
	currentAudioSample, audioStream.codecContext->time_base) <= 0) {
		// Encode video
		int rgbStride[1] = { 3*renderWidth };
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		renderPixels = malloc(3*renderHeight*renderWidth*sizeof(unsigned char));
		glReadPixels(0, 0, renderWidth, renderHeight, GL_RGB, GL_UNSIGNED_BYTE, renderPixels);
		av_frame_make_writable(videoStream.frame);
		int hr = sws_scale(swsContext, &renderPixels, rgbStride, 0, renderHeight,
			videoStream.frame->data, videoStream.frame->linesize);
		free(renderPixels);
		videoStream.frame->pts = currentVideoFrame++;
		if (!encode(videoStream)) {
			printf("Error encoding video frame\n");
			return;
		}
	} else {
		// Encode audio
		int16_t* a = (int16_t*)audioStream.tempFrame->data[0];
		for (int j = 0; j < audioStream.tempFrame->nb_samples; j++) {
			for (int i = 0; i < audioStream.codecContext->ch_layout.nb_channels; i++) {
				unsigned char v = (trackLength == 0 ? 0 :
					trackBuffer[audioStream.codecContext->ch_layout.nb_channels*currentAudioSample+i]);
				*a++ = v; // I'm sorry for this crime, it's how they do it in the samples
			}
			currentAudioSample++;
			audioStream.tempFrame->pts += audioStream.tempFrame->nb_samples;
		}
		int outSamples = av_rescale_rnd(swr_get_delay(swrContext,
			audioStream.codecContext->sample_rate) + audioStream.frame->nb_samples,
			audioStream.codecContext->sample_rate, audioStream.codecContext->sample_rate, AV_ROUND_UP);
		if (av_frame_make_writable(audioStream.frame) < 0) {
			printf("Error making audio frame writeable\n");
			return;
		}
		if (swr_convert(swrContext, audioStream.frame->data, outSamples,
		(const uint8_t **)audioStream.tempFrame->data, audioStream.tempFrame->nb_samples) < 0) {
			printf("Error while converting audio frame\n");
			return;
		}
		audioStream.frame->pts = av_rescale_q(audioStream.samples,
				(AVRational){1, audioStream.codecContext->sample_rate}, audioStream.codecContext->time_base);
		audioStream.samples += outSamples;
		if (!encode(audioStream)) {
			printf("Error encoding audio frame\n");
			return;
		}
	}
	if (currentVideoFrame >= maxVideoFrames) {
		av_write_trailer(outputContext);
		uninitVideoExport();
		saveVideo = 0;
		SDL_GL_SetSwapInterval(1);
		printf("Saved video\n");
	}
}

void exportFrame() {
	renderPixels = malloc(3*renderHeight*renderWidth*sizeof(unsigned char));
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, renderWidth, renderHeight, GL_BGR, GL_UNSIGNED_BYTE, renderPixels);
	SDL_Surface* saveSurf = SDL_CreateRGBSurfaceFrom(renderPixels, renderWidth, renderHeight, 24, 3*renderWidth, 0, 0, 0, 0);
	SDL_SaveBMP(saveSurf, "out/frame.bmp");
	SDL_FreeSurface(saveSurf);
	free(renderPixels);
	printf("Saved frame\n");
	saveFrame = 0;
}
