//
// Created by vagrant on 6/7/19.
//

#ifndef AGORA_CLEANSPEAK_AUDIOCONTAINER_H
#define AGORA_CLEANSPEAK_AUDIOCONTAINER_H

#include <string>
#include <vector>

#include "IAgoraLinuxSdkCommon.h"

typedef unsigned int uint_t;

struct InternalPcmFrame {
	InternalPcmFrame(const agora::linuxsdk::AudioPcmFrame& frame) :
			sampleRate(frame.sample_rates_),
			samples(frame.samples_) {}

	/** Bitrate of the sampling data.*/
	const uint_t sampleBits = 16; // 16
	/** Sampling rate.*/
	uint_t sampleRate; // 8k, 16k, 32k
	/** Number of samples of the frame.*/
	uint_t samples;
};

class AudioContainer {
	std::vector<InternalPcmFrame> buffers;

	std::vector<unsigned char> audio;

public:

	void append(const agora::linuxsdk::AudioPcmFrame& pcmAudio) {
		buffers.emplace_back(pcmAudio); // Copy the audio meta data (for debugging)
		audio.insert(audio.end(), pcmAudio.pcmBuf_, pcmAudio.pcmBuf_ + pcmAudio.pcmBufSize_);
	}

	std::string getFlacBase64() const;

	std::string getRawBase64() const;

	inline size_t size() const {
		return buffers.size();
	}

	inline void clear() {
		buffers.clear();
		audio.clear();
	}
};


#endif //AGORA_CLEANSPEAK_AUDIOCONTAINER_H
