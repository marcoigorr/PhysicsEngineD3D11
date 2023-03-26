#include "Engine.h"
#include <cmath>

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	_timer.Start();

	if (!this->render_window.CreateWnd(hInstance, window_title, window_class, width, height))
		return false;

	if (!this->gfx.Initialize(render_window.GetHWND(), width, height))
		return false;

	return true;
}

void Engine::CleanD3D()
{
	this->gfx.CleanD3D();
}

bool Engine::ProcessMessages()
{
	return this->render_window.ProcessMessages();
}

void Engine::Update()
{
	float dt = _timer.GetMillisecondElapsed();

	Entity* gravitySource = &gfx._entity[0];
	Entity* particle = &gfx._entity[1];

	float gravityStrength = -0.05f;

	XMVECTOR gravitySourceVec = gravitySource->GetPositionVector();
	XMFLOAT3 gravitySourceFloat3 = gravitySource->GetPositionFloat3();
	XMVECTOR particleVec = particle->GetPositionVector();
	XMFLOAT3 particleFloat3 = particle->GetPositionFloat3();

	float xDistance = particleFloat3.x - gravitySourceFloat3.x;
	float yDistance = particleFloat3.y - gravitySourceFloat3.y;
	float distance =  sqrtf(xDistance*xDistance + yDistance*yDistance);

	float inverseDistance = 1.0f / distance;
	float xNormalized = xDistance * inverseDistance;
	float yNormalized = yDistance * inverseDistance;

	float inverseSquareDropoff = inverseDistance * inverseDistance;
	float xAccelleration = xNormalized * gravityStrength * inverseSquareDropoff;
	float yAccelleration = yNormalized * gravityStrength * inverseSquareDropoff;

	static float xVelocity = 0.02f;
	static float yVelocity;

	xVelocity += xAccelleration * dt;
	yVelocity += yAccelleration * dt;

	particle->AdjustPosition(XMFLOAT3(xVelocity, yVelocity, 0.0f));

	_timer.Restart();
}

void Engine::RenderFrame()
{
	this->gfx.RenderFrame();
}