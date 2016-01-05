#include "AudioDevice.h"
#include <mmdeviceapi.h>
#include <vxLib/ScopeGuard.h>
#include <utility>

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

namespace Audio
{
	Device::Device()
		:m_ptr(nullptr)
	{

	}

	Device::~Device()
	{
		destroy();
	}

	bool Device::create()
	{
		if (m_ptr == nullptr)
		{
			IMMDeviceEnumerator *pEnumerator = nullptr;
			auto hr = CoCreateInstance(
				CLSID_MMDeviceEnumerator, NULL,
				CLSCTX_ALL, IID_IMMDeviceEnumerator,
				(void**)&pEnumerator);

			SCOPE_EXIT
			{
				if (pEnumerator)
				{
					pEnumerator->Release();
				}
			};

			if (hr != 0)
				return false;

			IMMDevice *pDevice = nullptr;
			hr = pEnumerator->GetDefaultAudioEndpoint(
				eRender, eConsole, &pDevice);
			if (hr != 0)
				return false;

			m_ptr = pDevice;
		}

		return true;
	}

	void Device::destroy()
	{
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = nullptr;
		}
	}
}