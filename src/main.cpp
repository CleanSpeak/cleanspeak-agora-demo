//
// Created by Tyler Scott on 2019-06-05.
//

#include <thread>
#include <chrono>

#include "IAgoraLinuxSdkCommon.h"
#include "IAgoraRecordingEngine.h"
#include "ModeratorHandler.h"

using namespace std;
using namespace agora::recording;

int main(int argc, char** argv) {
	ModeratorHandler handler;
	RecordingConfig config;

	config.isAudioOnly = true;
	config.isMixingEnabled = false;
	config.decodeAudio = agora::linuxsdk::AUDIO_FORMAT_TYPE::AUDIO_FORMAT_PCM_FRAME_TYPE;
	config.captureInterval = 5;
	config.audioIndicationInterval = 250;
	config.idleLimitSec = 3;

	// Bad practice, but had to be done.
	config.appliteDir = const_cast<char*>("/home/vagrant/CLionProjects/cleanspeak-agora-demo/bin");

	auto engine = IRecordingEngine::createAgoraRecordingEngine("ebdc95b9c8924407a43dc9b6b6b3756c", &handler);

	engine->joinChannel(nullptr, "1000", 0, config);

	// Gross, but is what the examples use (except i'm waiting a bit longer)
	while (!handler.isStopped()) {
		this_thread::sleep_for(100ms);
	}

	engine->release();

	this_thread::sleep_for(1s);

	return 0;
}
