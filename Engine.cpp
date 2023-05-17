#include "Engine.h"
#include <cmath>

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

	static Camera& s_camera = _gfx.GetCamera();
	static std::vector<Entity*>& s_particles = _gfx._particles;
	static QuadTreeNode* s_qtRoot = _gfx._qtRoot;

	if (_gfx._editing)
	{
		_timer.Restart();
		return true;
	}

	for (int i = 0; i < s_particles.size(); i++)
	{
		XMFLOAT2 acc = s_qtRoot->CalcForce(s_particles[i]);

		s_particles[i]->UpdateVelocity(acc.x, acc.y);
		s_particles[i]->AdjustPosition();
	}

	_timer.Restart();
	return true;
}

void Engine::RenderFrame()
{
	_gfx.RenderFrame();
}