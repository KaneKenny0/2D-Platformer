#include "sound.h"

sound::sound(const char* _filepath) {
	filepath = _filepath;
}

void sound::play() {
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		std::cout << "failed to initialise sound" << std::endl;
	}

	SDL_AudioSpec desiredSpec, obtainedSpec;
	SDL_memset(&desiredSpec, 0, sizeof(desiredSpec)); // Set desiredSpec to 0
	desiredSpec.freq = 44100;
	desiredSpec.format = AUDIO_S16SYS;
	desiredSpec.channels = 2;
	desiredSpec.samples = 4096;

	SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &obtainedSpec, SDL_AUDIO_ALLOW_ANY_CHANGE);

	SDL_AudioSpec soundSpec;
	Uint32 soundLength;
	Uint8* soundBuffer;

	if (SDL_LoadWAV("music.wav", &soundSpec, &soundBuffer, &soundLength) == NULL) {
		std::cout << "failed to load " << filepath << std::endl;
	}

	SDL_QueueAudio(audioDevice, soundBuffer, soundLength);
	SDL_PauseAudioDevice(audioDevice, 0);
	SDL_CloseAudioDevice(audioDevice);
}