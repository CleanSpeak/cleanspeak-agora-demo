//
// Created by Tyler Scott on 2019-06-05.
//

#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "IAgoraLinuxSdkCommon.h"
#include "ModeratorHandler.h"
#include "base64.h"

void ModeratorHandler::onAudioVolumeIndication(const agora::linuxsdk::AudioVolumeInfo speakers[],
                                               unsigned int speakerNum) {

	for(int i = 0; i < speakerNum; ++i) {
		active_users[speakers[i].uid] = 3;
	}

	for(auto& user: active_users) {
		if (user.second != 0) {
			user.second--;
		}
	}
}

void ModeratorHandler::onError(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code) {
	stopped = true;
}

void ModeratorHandler::onWarning(int warn) {
	// noop
}

void ModeratorHandler::onJoinChannelSuccess(const char* channelId, uid_t uid) {
	std::cout << "Connected" << std::endl;
}

void ModeratorHandler::onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code) {
	stopped = true;
}

void ModeratorHandler::onUserJoined(uid_t uid, agora::linuxsdk::UserJoinInfos &infos) {
	// noop
}

void ModeratorHandler::onUserOffline(uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) {
	active_users.erase(uid);
	previous_frame.erase(uid);
}

boost::uuids::uuid fromInt(uid_t i) {
	boost::uuids::uuid uuid = {{0}};

	// Push the user id into the uuid (lower 1/4th)
	*(uuid.begin() + 0) = static_cast<uint8_t>(i >> 24) & 0xFF;
	*(uuid.begin() + 1) = static_cast<uint8_t>(i >> 16) & 0xFF;
	*(uuid.begin() + 2) = static_cast<uint8_t>(i >> 8) & 0xFF;
	*(uuid.begin() + 3) = static_cast<uint8_t>(i >> 0) & 0xFF;
}

void ModeratorHandler::audioFrameReceived(unsigned int uid, const agora::linuxsdk::AudioFrame* frame) const {
	// TODO Handle tracking of frames and deciding when a user is active. Then send the data to cleanspeak if they are

	if (active_users.at(uid) == 0) {
		return; // This user hasn't talked recently enough to care.
	}

	if (frame->type != agora::linuxsdk::AUDIO_FRAME_TYPE::AUDIO_FRAME_AAC) {
		std::cerr << "Unexpected pcm frame!" << std::endl;
		return;
	}

	agora::linuxsdk::AudioAacFrame* aacFrame = frame->frame.aac;

	std::string audioUrl = "data:audio/aac;base64," + b64encode(aacFrame->aacBuf_, aacFrame->aacBufSize_);

	// Alternatively we could use the name_generator but its a hash, and we would lose the obviousness of the user id.
	std::string userId = to_string(fromInt(uid));

	std::string applicationId = "APPLICATION_ID";

	// language=JSON
	std::string request =
			R"(
{
  "content": {
    "applicationId": "",
    "createInstant": )" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()) + R"(,
    "location": "channel 1000",
    "parts": [
      {
        "content": ")" + audioUrl + R"(",
        "name": "audio",
        "type": "audio"
      }
    ],
    "senderId": ")" + userId + R"("
  }
}
)";

	CURL* curl;
	curl_slist* headers;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Authorization: API_KEY"); // TODO

	curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, "URL"); //TODO
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);

		curl_easy_setopt(curl, CURLOPT_READFUNCTION, [&](char* ptr, size_t size, size_t nitems, void* userdata) {
			// TODO
		});
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [&](char* ptr, size_t size, size_t nmemb, void* userdata) {
			// TODO
		});
		CURLcode res = curl_easy_perform(curl);
	}

	curl_slist_free_all(headers);
}

void ModeratorHandler::videoFrameReceived(unsigned int uid, const agora::linuxsdk::VideoFrame* frame) const {
	// noop
}

void ModeratorHandler::onActiveSpeaker(uid_t uid) {
	active_users[uid] = true;
}

void ModeratorHandler::onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed) {
	// noop
}

void ModeratorHandler::onFirstRemoteAudioFrame(uid_t uid, int elapsed) {
	// noop
}

void ModeratorHandler::onReceivingStreamStatusChanged(bool receivingAudio, bool receivingVideo) {
	if (receivingVideo) {
		std::cout << "For some reason we are receiving video now..." << std::endl;
	}
	if (!receivingAudio) {
		std::cerr << "We are no longer receiving audio!" << std::endl;
	}
}

void ModeratorHandler::onConnectionLost() {
	std::cout << "Connection lost" << std::endl;
	// Should we reconnect?
}

void ModeratorHandler::onConnectionInterrupted() {
	std::cout << "Connection interrupted" << std::endl;
	// Should we exit?
}
