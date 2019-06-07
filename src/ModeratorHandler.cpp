//
// Created by Tyler Scott on 2019-06-05.
//

#include <iostream>

#include "IAgoraLinuxSdkCommon.h"
#include "ModeratorHandler.h"
#include "CleanSpeakAudioModerationUtil.h"

// This is created in a static context because the recording api prevents internal fields from being modified when frames arrive... (const)
static CleanSpeakAudioModerationUtil moderationUtil;

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

//	for(int i = 0; i < speakerNum; ++i) {
//		users[speakers[i].uid].activeThreshold = 3;
//	}
//
//	for(auto& user: users) {
//		if (user.second.activeThreshold != 0) {
//			user.second.activeThreshold--;
//		}
//	}
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
	moderationUtil.addUser(uid);
}

void ModeratorHandler::onUserOffline(uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) {
	moderationUtil.removeUser(uid);
}

void ModeratorHandler::audioFrameReceived(uid_t uid, const agora::linuxsdk::AudioFrame* frame) const {
	moderationUtil.handleAudioFrames(uid, frame);
}

void ModeratorHandler::videoFrameReceived(uid_t uid, const agora::linuxsdk::VideoFrame* frame) const {
	// noop
}

void ModeratorHandler::onActiveSpeaker(uid_t uid) {
//	users[uid] = true;
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
