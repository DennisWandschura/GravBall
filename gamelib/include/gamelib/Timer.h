#pragma once

#include <vxLib/types.h>

class Timer
{
	f32 m_remainingTime;
	f32 m_time;

public:
	Timer() :m_remainingTime(0), m_time(0) {}
	explicit Timer(f32 time) :m_remainingTime(time), m_time(time) {}
	~Timer() {}

	bool update(f32 dt)
	{
		m_remainingTime -= dt;
		bool result = false;
		if (m_remainingTime <= 0.0f)
		{
			result = true;
			m_remainingTime = m_time;
		}

		return result;
	}
};