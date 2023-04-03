#pragma once
#include "WindowContainer.h"
#include "Timer.h"

class Engine : WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	void CleanD3D();
	bool ProcessMessages();
	bool Update();
	void RenderFrame();

private:
	Timer _timer;
};
