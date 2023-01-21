#include "Engine.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	if (!this->render_window.CreateWnd(hInstance, window_title, window_class, width, height))
		return false;

	if (!this->gfx.InitD3D(render_window.GetHWND(), width, height))
		return false;

	return true;
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::CleanD3D()
{
	this->gfx.CleanD3D();
}

void Engine::RenderFrame()
{
	this->gfx.RenderFrame();
}