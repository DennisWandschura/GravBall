#pragma once

#include <vxLib/types.h>

namespace Event
{
	class Event;

	class ListenerInterface
	{
	public:
		virtual ~ListenerInterface() {}

		virtual void handleEvent(const Event &evt) = 0;
	};
}