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
	static float xVelocity = 0.03f;
	static float yVelocity;
	static float gravityStrength = -0.035f;

	XMFLOAT3 gravitySourceFloat3 = gravitySource->GetPositionFloat3();
	for (Entity* particle : _gfx._particles)
	{
		if (particle->isBeingEdited)
			continue;

		XMFLOAT3 particleFloat3 = particle->GetPositionFloat3();

		float xDistance = particleFloat3.x - gravitySourceFloat3.x;
		float yDistance = particleFloat3.y - gravitySourceFloat3.y;
		float distance = sqrtf(xDistance * xDistance + yDistance * yDistance);

		float inverseDistance = 1.0f / distance;
		float xNormalized = xDistance * inverseDistance;
		float yNormalized = yDistance * inverseDistance;

		float inverseSquareDropoff = inverseDistance * inverseDistance;
		float xAccelleration = xNormalized * gravityStrength * inverseSquareDropoff;
		float yAccelleration = yNormalized * gravityStrength * inverseSquareDropoff;

		xVelocity += xAccelleration * dt;
		yVelocity += yAccelleration * dt;

		particle->AdjustPosition(XMFLOAT3(xVelocity, yVelocity, 0.0f));
	}
	

	_timer.Restart();
	return true;
}

void Engine::RenderFrame()
{
	_gfx.RenderFrame();
}