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

	/*if (_gfx._particles->isBeingEdited)
	{
		_timer.Restart();
		return true;
	}*/

	static Entity* gravitySource = &_gfx._gravitySource;
	static float gravityStrength = -0.3f;

	for (int i = 0; i < ARRAYSIZE(_gfx._particles); i++)
	{
		XMFLOAT3 gravitySourceFloat3 = gravitySource->GetPositionFloat3();
		XMFLOAT3 particleFloat3 = _gfx._particles[i]->GetPositionFloat3();

		float xDistance = particleFloat3.x - gravitySourceFloat3.x;
		float yDistance = particleFloat3.y - gravitySourceFloat3.y;
		float distance = sqrtf(xDistance * xDistance + yDistance * yDistance);

		float inverseDistance = 1.0f / distance;
		float xNormalized = xDistance * inverseDistance;
		float yNormalized = yDistance * inverseDistance;

		float inverseSquareDropoff = inverseDistance * inverseDistance;
		float xAccelleration = xNormalized * gravityStrength * inverseSquareDropoff;
		float yAccelleration = yNormalized * gravityStrength * inverseSquareDropoff;
		
		_gfx._particles[i]->UpdateVelocity(xAccelleration * dt, yAccelleration * dt);

		// Collision

		_gfx._particles[i]->AdjustPosition();
	}

	_timer.Restart();
	return true;
}

void Engine::RenderFrame()
{
	_gfx.RenderFrame();
}