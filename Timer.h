#pragma once
#include <chrono>

class Timer
{
public:
	Timer();
	double GetMillisecondElapsed();
	void Restart();
	bool Stop();
	bool Start();

private:
	bool _isRunnung = false;

	std::chrono::time_point<std::chrono::steady_clock> _start;
	std::chrono::time_point<std::chrono::steady_clock> _stop;

};