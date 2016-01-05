#pragma once

struct IXAudio2;
struct IXAudio2MasteringVoice;

#include <vxLib/types.h>

class AudioAspect
{
	IXAudio2MasteringVoice* m_master;
	IXAudio2* m_device;

public:
	AudioAspect();
	~AudioAspect();

	bool initialize();
	void shutdown();

	void update();
};