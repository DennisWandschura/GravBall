#include "AudioAspect.h"
#include <xaudio2.h>

AudioAspect::AudioAspect()
	:m_master(nullptr),
	m_device(nullptr)
{

}

AudioAspect::~AudioAspect()
{

}

bool AudioAspect::initialize()
{
	IXAudio2* pXAudio2 = NULL;
	HRESULT hr;
	if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR)))
		return false;

	m_device = pXAudio2;
	hr = m_device->CreateMasteringVoice(&m_master);
	if (FAILED(hr))
		return false;

	return true;
}

void AudioAspect::shutdown()
{
	if (m_master)
	{
		
	}

	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}
}

void AudioAspect::update()
{

}