//
// Created by Tyler Scott on 2019-06-05.
//

#ifndef AGORA_CLEANSPEAK_MODERATORHANDLER_H
#define AGORA_CLEANSPEAK_MODERATORHANDLER_H

#include <unordered_map>
#include <atomic>

#include "IAgoraRecordingEngine.h"
#include "IAgoraLinuxSdkCommon.h"

class ModeratorHandler: public agora::recording::IRecordingEngineEventHandler{
private:
	std::atomic_bool stopped;

	std::unordered_map<uid_t, int> active_users;

	std::unordered_map<uid_t, char*> previous_frame;

public:

	ModeratorHandler(): stopped(false) {
	}

	bool isStopped() const {
		return stopped;
	}

	virtual void onAudioVolumeIndication(const agora::linuxsdk::AudioVolumeInfo* speakers,
	                                     unsigned int speakerNum) override;

	virtual void onError(int error, agora::linuxsdk::STAT_CODE_TYPE stat_code) override;

	virtual void onWarning(int warn) override;

	virtual void onJoinChannelSuccess(const char* channelId, uid_t uid) override;

	virtual void onLeaveChannel(agora::linuxsdk::LEAVE_PATH_CODE code) override;

	virtual void onUserJoined(uid_t uid, agora::linuxsdk::UserJoinInfos &infos) override;

	virtual void onUserOffline(uid_t uid, agora::linuxsdk::USER_OFFLINE_REASON_TYPE reason) override;

	virtual void audioFrameReceived(unsigned int uid, const agora::linuxsdk::AudioFrame* frame) const override;

	virtual void videoFrameReceived(unsigned int uid, const agora::linuxsdk::VideoFrame* frame) const override;

	virtual void onActiveSpeaker(uid_t uid) override;

	virtual void onFirstRemoteVideoDecoded(uid_t uid, int width, int height, int elapsed) override;

	virtual void onFirstRemoteAudioFrame(uid_t uid, int elapsed) override;

	virtual void onReceivingStreamStatusChanged(bool receivingAudio, bool receivingVideo) override;

	virtual void onConnectionLost() override;

	virtual void onConnectionInterrupted() override;
};


#endif //AGORA_CLEANSPEAK_MODERATORHANDLER_H
