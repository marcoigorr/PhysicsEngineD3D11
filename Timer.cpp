#include "Timer.h"

Timer::Timer()
{
	_start = std::chrono::high_resolution_clock::now();
	_stop = std::chrono::high_resolution_clock::now();
}

double Timer::GetMillisecondElapsed()
{
	if (_isRunnung)
	{
		auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - _start);
		return elapsed.count();
	}
	else
	{
		auto elapsed = std::chrono::duration<double, std::milli>(_stop - _start);
		return elapsed.count();
	}
}

void Timer::Restart()
{
	_isRunnung = true;
	_start = std::chrono::high_resolution_clock::now();
}

bool Timer::Stop()
{
	if (!_isRunnung)
		return false;

	_stop = std::chrono::high_resolution_clock::now();
	_isRunnung = false;
	return true;
}

bool Timer::Start()
{
	if (_isRunnung)
		return false;

	_start = std::chrono::high_resolution_clock::now();
	_isRunnung = true;
	return true;
}