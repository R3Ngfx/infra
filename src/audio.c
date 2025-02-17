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

#define BUFFER_SIZE 8192
#define FFT_SIZE BUFFER_SIZE/2

fftwf_complex* outputBuffer;
unsigned int audioBufferPos = 0;
float audioBuffer[BUFFER_SIZE];
float hann[BUFFER_SIZE];
float blackmanHarris[BUFFER_SIZE];

// Given a frequency in Hz return the index of its position in the FFT output array
int freqToIndex(int f) {
	return (int)(BUFFER_SIZE*f/(float)outputSampleRate);
}

void updateFFT(float* inputBuffer) {
	// Allocate variables
	float audioCurrent[8] = {0};
	// Apply windowing
	for (int i = 0; i < BUFFER_SIZE; i++) {
		//inputBuffer[i] *= hann[i];
		inputBuffer[i] *= blackmanHarris[i];
	}
	// Calculate FFT
	fftwf_plan plan = fftwf_plan_dft_r2c_1d(BUFFER_SIZE, inputBuffer, outputBuffer, FFTW_ESTIMATE);
	fftwf_execute(plan);
	fftwf_destroy_plan(plan);
	// Obtain current visualization values
	int splits[] = {freqToIndex(60), freqToIndex(250), freqToIndex(500),
		freqToIndex(2000), freqToIndex(4000), freqToIndex(6000), freqToIndex(20000)};
	int currentSplit = 0;
	for (int i = freqToIndex(20); i < freqToIndex(20000); i++) {
		double a = outputBuffer[i][0];
		double b = outputBuffer[i][1];
		float r = sqrt(a*a+b*b);
		if (i > splits[currentSplit]) currentSplit++;
		audioCurrent[currentSplit] += r;
		audioCurrent[7] += r;
	}
	// Update values
	for (int i = 0; i < 8; i++) {
		audioMax[i] = max(audioMax[i]*(1-0.01*drop), audioCurrent[i]);
		if (audioMax[i] > 0) audioNormalized[i] = audioCurrent[i]/audioMax[i];
	}
}

void dataCallback(ma_device* callbackDevice, void* callbackOutput, const void* callbackInput, ma_uint32 frameCount) {
	if (saveVideo) return;
	// Play sound
	ma_decoder* callbackDecoder = (ma_decoder*)callbackDevice->pUserData;
	if (callbackDecoder == NULL) return;
	ma_decoder_read_pcm_frames(callbackDecoder, callbackOutput, frameCount);
	(void)callbackInput;
}

void unloadTrack() {
	if (trackLength == 0) return;
	playing = 0;
	ma_device_stop(&device);
	ma_device_uninit(&device);
	ma_decoder_uninit(&decoder);
	SDL_FreeWAV(trackBuffer);
}

int loadTrack(char* path) {
	unloadTrack();
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
	for (int i = 0; i < 8; i++) {
		audioMax[i] = 0;
	}
	return 1;
}

void initAudio() {
	// Precalculate Hann window
	float N = BUFFER_SIZE;
	for (int n = 0; n < BUFFER_SIZE; n++) {
		float r = sin(Pi*n/(float)N);
		hann[n] = r*r;
		blackmanHarris[n] = 0.35875 - 0.48829*cos(2*Pi*n/N) + 0.14128*cos(4*Pi*n/N) - 0.01168*cos(6*Pi*n/N);
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
	if (lastRenderedTime != currentTime || saveFrame || saveVideo) {
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
		float audioCurrent[8];
		for (int i = 0; i < 8; i++) {
			audioCurrent[i] = pow(audioNormalized[i], power);
			audio[i] = lerp(audioCurrent[i], audio[i], smoothness);
			audioInc[i] += 0.1*audio[i];

		}
	}
}
