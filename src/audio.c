// Audio functions and visualization
// Author: Mateo Vallejo
// License: GNU General Public License (See LICENSE for full details)

#define MINIAUDIO_IMPLEMENTATION
#include "global.c"
#include "miniaudio/miniaudio.h"
#include <fftw3.h>

ma_decoder decoder;
ma_device_config deviceConfig;
ma_device device;

#define BUFFER_SIZE 2048
#define FFT_SIZE BUFFER_SIZE/2

fftwf_complex* outputBuffer;
unsigned int audioBufferPos = 0;
float audioBuffer[BUFFER_SIZE];
float hann[BUFFER_SIZE];

// Given a frequency in Hz return the index of its position in the FFT output array
int freqToIndex(int f) {
	return (int)(BUFFER_SIZE*f/(float)outputSampleRate);
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
	maxLows = max(currentLows, maxLows);
	maxMids = max(currentMids, maxMids);
	maxHighs = max(currentHighs, maxHighs);
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
	(void)callbackInput;
}

int loadTrack(char* path) {
	// Load SDL track for rendering
	if (SDL_LoadWAV(path, &spec, &trackBuffer, &trackLength) == NULL) {
		warning("Error loading audio file");
		return 0;
	}
	trackSampleRate = spec.freq;
	outputSampleRate = trackSampleRate;
	trackChannels = spec.channels;
	trackDuration = trackLength/trackSampleRate/trackChannels/(SDL_AUDIO_BITSIZE(spec.format)/8);
	// Load miniaudio track for playback
	if (ma_decoder_init_file(path, NULL, &decoder) != MA_SUCCESS) {
		warning("Error loading audio file");
		return 0;
	}
	deviceConfig = ma_device_config_init(ma_device_type_playback);
	deviceConfig.playback.format = decoder.outputFormat;
	deviceConfig.playback.channels = decoder.outputChannels;
	deviceConfig.sampleRate = decoder.outputSampleRate;
	deviceConfig.dataCallback = dataCallback;
	deviceConfig.pUserData = &decoder;
	if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
		warning("Error initializing audio device");
		return 0;
	}
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
		float r = sin(3.14159265358979323846*n/(float)BUFFER_SIZE);
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
			warning("Error starting audio device");
			// TODO: Handle error
		}
	} else {
		if (ma_device_stop(&device) != MA_SUCCESS) {
			warning("Error stopping audio device");
			// TODO: Handle error
		}
	}
}

void seekAudio() {
	if (trackLength == 0) return;
	ma_decoder_seek_to_pcm_frame(&decoder, currentTime*outputSampleRate);
}

void renderAudio() {
	if (trackLength == 0) return;
	// Fill track buffer with current sound
	int idx = currentTime*trackSampleRate-BUFFER_SIZE;
	idx = clamp(0, (trackDuration*trackSampleRate)-BUFFER_SIZE, idx);
	for (int i = 0; i < BUFFER_SIZE; i++) {
		float v = 0;
		for (int c = 0; c < trackChannels; c++) {
			v = max(v, getTrackSample(idx+i, c));
		}
		audioBuffer[i] = v;
	}
	// Update fft with new input buffer
	updateFFT(audioBuffer);
	// Drop max values with clamping
	maxLows = max(maxLows-deltaTime*drop, 0);
	maxMids = max(maxMids-deltaTime*drop, 0);
	maxHighs = max(maxHighs-deltaTime*drop, 0);
	// Assign final smoothed values
	lows = lerp(lows, pow(normalizedLows, power), deltaTime*smoothness);
	mids = lerp(mids, pow(normalizedMids, power), deltaTime*smoothness);
	highs = lerp(highs, pow(normalizedHighs, power), deltaTime*smoothness);
}
