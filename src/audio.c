// Audio functions and visualization
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#define MINIAUDIO_IMPLEMENTATION
#include "global.c"
#include "miniaudio/miniaudio.h"
#include <fftw3.h>

SDL_AudioSpec spec;
unsigned int trackLength = 0;
unsigned char* trackBuffer;

ma_decoder decoder;
ma_device_config deviceConfig;
ma_device device;

#define SAMPLE_RATE 48000
#define BUFFER_SIZE 2048
#define FFT_SIZE BUFFER_SIZE/2

fftwf_complex* outputBuffer;
unsigned int audioBufferPos = 0;
float audioBuffer[BUFFER_SIZE];
float hann[BUFFER_SIZE];

// Given a frequency in Hz return the index of its position in the FFT output array
int freqToIndex(int f) {
	return (int)(BUFFER_SIZE*f/(float)SAMPLE_RATE);
}

void updateFFT(float* inputBuffer) {
	// Allocate variables
	float currentLows = 0, currentMids = 0, currentHighs = 0;
	// Apply windowing
	for (int i = 0; i < BUFFER_SIZE; i++) {
		inputBuffer[i] *= hann[i];
	}
	// Calculate FFT
	fftwf_plan plan = fftwf_plan_dft_r2c_1d(BUFFER_SIZE, inputBuffer, outputBuffer, FFTW_ESTIMATE);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
	// Obtain current visualization values
	for (int i = freqToIndex(20); i < freqToIndex(20000); i++) {
		double a = outputBuffer[i][0];
		double b = outputBuffer[i][1];
		float r = sqrt(a*a+b*b);
		if (i < freqToIndex(250)) {
			currentLows += r;
		} else if (i < freqToIndex(2000)) {
			currentMids += r;
		} else {
			currentHighs += r;
		}
	}
	// Update max
	maxLows = currentLows > maxLows ? currentLows : maxLows;
	maxMids = currentMids > maxMids ? currentMids : maxMids;
	maxHighs = currentHighs > maxHighs ? currentHighs : maxHighs;
	// Check division by 0 to avoid NaNs
	if (maxLows > 0) normalizedLows = currentLows/maxLows;
	if (maxMids > 0) normalizedMids = currentMids/maxMids;
	if (maxHighs > 0) normalizedHighs = currentHighs/maxHighs;
}

void dataCallback(ma_device* callbackDevice, void* callbackOutput, const void* callbackInput, ma_uint32 frameCount) {
	if (saveVideo) return;
	// Play sound
	ma_decoder* callbackDecoder = (ma_decoder*)callbackDevice->pUserData;
	if (callbackDecoder == NULL) return;
	ma_decoder_read_pcm_frames(callbackDecoder, callbackOutput, frameCount);
	// Do visualization with output audio if not rendering
	if (!saveVideo && playing) {
		for (int i = 0; i < frameCount; i++) {
			audioBuffer[audioBufferPos] = ((float*)callbackOutput)[i];
			audioBufferPos++;
			if (audioBufferPos == BUFFER_SIZE) {
				audioBufferPos = 0;
				updateFFT(audioBuffer);
			}
		}
	}
	(void)callbackInput;
}

int loadTrack(char* path) {
	// Load miniaudio track for playback
	if (ma_decoder_init_file(path, NULL, &decoder) != MA_SUCCESS) {
		printf("Error loading audio file\n");
		return 0;
	}
	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = decoder.outputFormat;
	deviceConfig.playback.channels = decoder.outputChannels;
	deviceConfig.sampleRate = SAMPLE_RATE;
	deviceConfig.dataCallback = dataCallback;
	deviceConfig.pUserData = &decoder;
	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		printf("Error initializing audio device\n");
		return 0;
	}
	// Load SDL track for rendering
	SDL_LoadWAV(path, &spec, &trackBuffer, &trackLength);
	return 1;
}

void unloadTrack() {
	if (trackLength == 0) return;
	ma_device_uninit(&device);
	ma_decoder_uninit(&decoder);
	SDL_FreeWAV(trackBuffer);
}

void initAudio() {
	// Precalculate Hann window
	for (int n = 0; n < BUFFER_SIZE; n++) {
		float r = sin(pi*n/(float)BUFFER_SIZE);
		hann[n] = r*r;
	}
	outputBuffer = (fftwf_complex*)fftwf_malloc(sizeof(fftw_complex)*((FFT_SIZE)));
}

void uninitAudio() {
	unloadTrack();
	fftw_free(outputBuffer);
}

void playPauseAudio() {
	if (trackLength == 0) return;
	if (playing) {
		if (ma_device_start(&device) != MA_SUCCESS) {
			printf("Error starting audio device\n");
			// TODO: Handle error
		}
	} else {
		if (ma_device_stop(&device) != MA_SUCCESS) {
			printf("Error stopping audio device\n");
			// TODO: Handle error
		}
	}
}

void seekAudio() {
	if (trackLength == 0) return;
	ma_decoder_seek_to_pcm_frame(&decoder, currentTime*SAMPLE_RATE);
}

void renderAudio(int offline) {
	if (trackLength == 0) return;
	if (offline) {
		// Get starting index of current pcm frame window
		int idx = currentTime*SAMPLE_RATE-BUFFER_SIZE;
		ma_uint64 length = 0;
		ma_data_source_get_length_in_pcm_frames(&decoder, &length);
		idx = clamp(0, length, idx);
		// Read pcm frames from data source
		ma_data_source_seek_to_pcm_frame(&decoder, idx);
		ma_decoder_read_pcm_frames(&decoder, audioBuffer, BUFFER_SIZE);
		ma_uint64 framesRead = 0;
		ma_data_source_read_pcm_frames(&decoder, audioBuffer, BUFFER_SIZE, &framesRead, 0);
		// Update fft with new input buffer
		updateFFT(audioBuffer);
		if (!saveVideo) {
			lows = normalizedLows;
			mids = normalizedMids;
			highs = normalizedHighs;
			return;
		}
	}
	// Drop max values
	maxLows -= deltaTime*drop;
	maxMids -= deltaTime*drop;
	maxHighs -= deltaTime*drop;
	// Assign final smoothed values
	lows = lerp(lows, pow(normalizedLows, power), deltaTime*smoothness);
	mids = lerp(mids, pow(normalizedMids, power), deltaTime*smoothness);
	highs = lerp(highs, pow(normalizedHighs, power), deltaTime*smoothness);
}
