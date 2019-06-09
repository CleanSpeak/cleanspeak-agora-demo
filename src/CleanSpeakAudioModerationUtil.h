//
// Created by vagrant on 6/7/19.
//

#ifndef AGORA_CLEANSPEAK_CLEANSPEAKAUDIOMODERATIONUTIL_H
#define AGORA_CLEANSPEAK_CLEANSPEAKAUDIOMODERATIONUTIL_H

#include <unordered_map>

#include "IAgoraLinuxSdkCommon.h"
#include "AudioContainer.h"

class CleanSpeakAudioModerationUtil {

	struct UserData {
		//	int activeThreshold = 0;

		AudioContainer buffers;
	};

	std::unordered_map<uid_t, UserData> users;

public:

	CleanSpeakAudioModerationUtil();

	~CleanSpeakAudioModerationUtil();

	void handleAudioFrames(uid_t uid, const agora::linuxsdk::AudioFrame* frame);

	void addUser(uid_t id);

	void removeUser(uid_t id);
};


#endif //AGORA_CLEANSPEAK_CLEANSPEAKAUDIOMODERATIONUTIL_H
