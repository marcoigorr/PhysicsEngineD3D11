#pragma once
#include "ErrorLogger.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

class ImGuiWindow
{
public:
	bool Initialize(HWND hWnd, ID3D11Device* dev, ID3D11DeviceContext* devcon);
	void ShutDown();
	void BeginRender();
	void EndRender();
};
