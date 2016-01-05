#pragma once

struct IMMDevice;

#include <vxLib/types.h>

namespace Audio
{
	class Device
	{
		IMMDevice* m_ptr;

	public:
		Device();
		~Device();

		bool create();
		void destroy();

		IMMDevice* get() { return m_ptr; }

		IMMDevice* operator->()
		{
			return m_ptr;
		}
	};
}