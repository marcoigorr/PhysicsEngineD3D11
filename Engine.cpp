#include "Engine.h"

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

	{
		Entity* entRef = &gfx._entity;
		Camera* camRef = &gfx._camera;
		DirectX::XMFLOAT3 entPos = entRef->GetPositionFloat3();
		DirectX::XMFLOAT3 cameraPos = camRef->GetPositionFloat3();

		float vx = 0.0f;
		float vy = -0.015f;

		if (entPos.y * -1 >= entPos.z)
			vy *= -1;
			entRef->AdjustPosition(vx, vy * dt, 0.0f);

		entRef->AdjustPosition(vx, vy * dt, 0.0f);	
	}

	_timer.Restart();
}

void Engine::RenderFrame()
{
	this->gfx.RenderFrame();
}