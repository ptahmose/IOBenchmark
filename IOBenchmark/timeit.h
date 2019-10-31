#pragma once

#include <Windows.h>

class CTimeIt
{
private:
	LARGE_INTEGER tickStarted;
	LARGE_INTEGER tickStopped;
public:
	CTimeIt()
	{
		QueryPerformanceCounter(&this->tickStarted);
	}

	void Stop()
	{
		QueryPerformanceCounter(&this->tickStopped);
	}

	double GetElapsedTimeInMilliseconds() const
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);

		double time = ((double)(this->tickStopped.QuadPart - this->tickStarted.QuadPart)) / (freq.QuadPart / 1000.0);
		return time;
	}

	double GetElapsedTimeInSeconds() const
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);

		double time = ((double)(this->tickStopped.QuadPart - this->tickStarted.QuadPart)) / (freq.QuadPart);
		return time;
	}
};
