//
// Created by Tyler Scott on 2019-06-05.
//

#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

#include "IAgoraLinuxSdkCommon.h"
#include "ModeratorHandler.h"
#include "base64.h"

std::ostream& operator<<(std::ostream& out, const agora::linuxsdk::STAT_CODE_TYPE value){
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch(value){
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_OK)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_ERR_FROM_ENGINE)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_ERR_ARS_JOIN_CHANNEL)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_ERR_CREATE_PROCESS)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_ERR_MIXED_INVALID_VIDEO_PARAM)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_ERR_NULL_POINTER)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_ERR_PROXY_SERVER_INVALID_PARAM)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_POLL_ERR)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_POLL_HANG_UP)
		PROCESS_VAL(agora::linuxsdk::STAT_CODE_TYPE::STAT_POLL_NVAL)
	}
#undef PROCESS_VAL

	return out << s;
}

std::ostream &operator<<(std::ostream &out, const agora::linuxsdk::LEAVE_PATH_CODE value) {
	const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
	switch (value) {
		PROCESS_VAL(agora::linuxsdk::LEAVE_PATH_CODE::LEAVE_CODE_INIT)
		PROCESS_VAL(agora::linuxsdk::LEAVE_PATH_CODE::LEAVE_CODE_SIG)
		PROCESS_VAL(agora::linuxsdk::LEAVE_PATH_CODE::LEAVE_CODE_NO_USERS)
		PROCESS_VAL(agora::linuxsdk::LEAVE_PATH_CODE::LEAVE_CODE_TIMER_CATCH)
		PROCESS_VAL(agora::linuxsdk::LEAVE_PATH_CODE::LEAVE_CODE_CLIENT_LEAVE)
	}
#undef PROCESS_VAL

	return out << s;
}

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
	std::cerr << "Received error from Agora [" << stat_code << "] Stopping..." << std::endl;
	std::cerr.flush();
	stopped = true;
}

void ModeratorHandler::onWarning(int warn) {
	// noop
}

void ModeratorHandler::onJoinChannelSuccess(const char* channelId, uid_t uid) {
	std::cout << "Connected" << std::endl;
}

void ModeratorHandler::onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code) {
	std::cerr << "Left the channel! Code [" << code << "]. Stopping..." << std::endl;
	std::cerr.flush();
	stopped = true;
}

void ModeratorHandler::onUserJoined(uid_t uid, agora::linuxsdk::UserJoinInfos &infos) {
	active_users[uid] = 0;
}

void ModeratorHandler::onUserOffline(uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) {
	active_users.erase(uid);
	previous_frame.erase(uid);
}

boost::uuids::uuid fromInt(uid_t i) {
	boost::uuids::uuid uuid = {{0}};

	// Push the user id into the uuid (lower 1/4th)
	*(uuid.begin() + 12) = static_cast<uint8_t>(i >> 24) & 0xFF;
	*(uuid.begin() + 13) = static_cast<uint8_t>(i >> 16) & 0xFF;
	*(uuid.begin() + 14) = static_cast<uint8_t>(i >> 8) & 0xFF;
	*(uuid.begin() + 15) = static_cast<uint8_t>(i >> 0) & 0xFF;
}

static size_t requestBodyHandler(char* ptr, size_t size, size_t nitems, std::stringbuf* body) {
	return body->sgetn(ptr, nitems);
}

static size_t responseBodyHandler(char* ptr, size_t size, size_t nmemb, std::stringbuf* responseBody) {
	responseBody->sputn(ptr, nmemb);
	return size * nmemb;
}

void ModeratorHandler::audioFrameReceived(uid_t uid, const agora::linuxsdk::AudioFrame* frame) const {
	// TODO Handle tracking of frames and deciding when a user is active. Then send the data to cleanspeak if they are

	if (active_users.find(uid) != active_users.end() && active_users.at(uid) == 0) {
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

	std::string applicationId = "c88755c8-7789-4b28-8f0d-180088772e55";

	// language=JSON
	std::string request =
			R"(
{
  "content": {
    "applicationId": ")" + applicationId + R"(",
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

	CURL* curl = nullptr;
	curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Authorization: 2S-Wx_U-VgfTRhzYmav_hHna54YqdgHERB9T3vvzV28");

	curl = curl_easy_init();

	if (curl) {

		CURLcode code;

//		code = curl_easy_setopt(curl, CURLOPT_VERBOSE, true);

		code = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		code = curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8001/content/item/moderate");
		code = curl_easy_setopt(curl, CURLOPT_POST, true);

		std::stringbuf responseBody;
		std::stringbuf body(request);

		// Handles the request body
		code = curl_easy_setopt(curl, CURLOPT_READDATA, &body);
		code = curl_easy_setopt(curl, CURLOPT_READFUNCTION, requestBodyHandler);
		// Handles the response body
		code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
		code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseBodyHandler);

		CURLcode res = curl_easy_perform(curl);

		if (res == CURLE_OK) {
		    long http_code = 0;
		    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		    if (http_code != 200) {
		        std::cerr << "Something has gone wrong with the request! (HTTP CODE) [" << http_code << "]" << std::endl
		        << "Body [" << responseBody.str() << "]" << std::endl;
		    }
		} else {
		    std::cerr << "Something went wrong during the request! (CURL ERROR)" << std::endl;
		}
	}

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
}

void ModeratorHandler::videoFrameReceived(uid_t uid, const agora::linuxsdk::VideoFrame* frame) const {
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

ModeratorHandler::ModeratorHandler() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

ModeratorHandler::~ModeratorHandler() {
	curl_global_cleanup();
}
