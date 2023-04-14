#include "Engine.h"
#include <cmath>

#define BIG_G 6.673e-11

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	_timer.Start();

	if (!this->_renderWindow.CreateWnd(hInstance, window_title, window_class, width, height))
		return false;

	if (!this->_gfx.Initialize(_renderWindow.GetHWND(), width, height))
		return false;

	return true;
}

void Engine::CleanD3D()
{
	_gfx.CleanD3D();
}

bool Engine::ProcessMessages()
{
	return _renderWindow.ProcessMessages();
}

bool Engine::Update()
{
	float dt = _timer.GetMillisecondElapsed();

	if (_gfx._editing)
	{
		_timer.Restart();
		return true;
	}

	/* N Squared order for gravity computation
	 *
	 *for (int i = 0; i < _gfx._particles.size(); i++)
	{
		for (int j = 0; j < _gfx._particles.size(); j++)
		{
			if (i != j)
				_gfx._particles[i]->Attract(_gfx._particles[j], dt);
		}
	}*/

	_timer.Restart();
	return true;
}

void Engine::RenderFrame()
{
	_gfx.RenderFrame();
}