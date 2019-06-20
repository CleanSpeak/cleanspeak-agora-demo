//
// Created by vagrant on 6/7/19.
//

#include <iostream>
#include <curl/curl.h>
#include <chrono>
#include <sstream>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>
#include <vector>
#include <IAgoraLinuxSdkCommon.h>

#include "CleanSpeakAudioModerationUtil.h"
#include "AudioContainer.h"
#include "IAgoraLinuxSdkCommon.h"

static boost::uuids::uuid fromInt(uid_t id) {
	boost::uuids::uuid uuid = {{0}};

	// Push the user id into the uuid (lower 1/4th)
	*(uuid.begin() + 12) = static_cast<uint8_t>(id >> 24) & 0xFF;
	*(uuid.begin() + 13) = static_cast<uint8_t>(id >> 16) & 0xFF;
	*(uuid.begin() + 14) = static_cast<uint8_t>(id >> 8) & 0xFF;
	*(uuid.begin() + 15) = static_cast<uint8_t>(id >> 0) & 0xFF;

	return uuid;
}

static size_t requestBodyHandler(char* ptr, size_t size, size_t nitems, std::stringbuf* body) {
	return body->sgetn(ptr, nitems);
}

static size_t responseBodyHandler(char* ptr, size_t size, size_t nmemb, std::stringbuf* responseBody) {
	responseBody->sputn(ptr, nmemb);
	return size * nmemb;
}

void CleanSpeakAudioModerationUtil::handleAudioFrames(uid_t uid, const agora::linuxsdk::AudioFrame* frame) {
	if (frame->type != agora::linuxsdk::AUDIO_FRAME_TYPE::AUDIO_FRAME_RAW_PCM) {
		std::cerr << "Unexpected aac frame!" << std::endl;
		return;
	}

	UserData& userData = users[uid];
	userData.buffers.append(*(frame->frame.pcm));

	if (userData.buffers.size() <
	    frame->frame.pcm->sample_rates_ / frame->frame.pcm->samples_ * 60) { // Sample every 60 seconds
		return;
	}

//	std::string audioUrl = "data:audio/flac;base64," + userData.buffers.getFlacBase64();
	std::string audioUrl = "data:audio/pcm;base64," + userData.buffers.getRawBase64();

	userData.buffers.clear(); // Reset for next chunk

	// Alternatively we could use the name_generator but its a hash, and we would lose the obviousness of the user id.
	std::string userId = to_string(fromInt(uid));

	std::string applicationId = "e8f74714-7c02-41e9-9fab-82f9cb2829be";

	std::string request =
			R"(
{
  "content": {
    "applicationId": ")" + applicationId + R"(",
    "createInstant": )" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()).count()) + R"(,
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
	headers = curl_slist_append(headers, ("Content-Length: " + std::to_string(request.length())).c_str());
	headers = curl_slist_append(headers, "Authorization: 2S-Wx_U-VgfTRhzYmav_hHna54YqdgHERB9T3vvzV28");

	curl = curl_easy_init();

	if (curl) {

//		curl_easy_setopt(curl, CURLOPT_VERBOSE, true);

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8001/content/item/moderate");
		curl_easy_setopt(curl, CURLOPT_POST, true);

		std::stringbuf responseBody;
		std::stringbuf body(request);

		// Handles the request body
		curl_easy_setopt(curl, CURLOPT_READDATA, &body);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, requestBodyHandler);
		// Handles the response body
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, responseBodyHandler);

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

CleanSpeakAudioModerationUtil::CleanSpeakAudioModerationUtil() {
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

CleanSpeakAudioModerationUtil::~CleanSpeakAudioModerationUtil() {
	curl_global_cleanup();
}

void CleanSpeakAudioModerationUtil::addUser(uid_t id) {
	users[id] = UserData();
}

void CleanSpeakAudioModerationUtil::removeUser(uid_t id) {
	users.erase(id);
}
