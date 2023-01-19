#pragma once

class Direct3D11
{
public:
	IDXGISwapChain* swapchain;             
	ID3D11Device* dev;                     
	ID3D11DeviceContext* devcon;

	void InitD3D(HWND hWnd);     // sets up and initializes Direct3D
	void CleanD3D(void);         // closes Direct3D and releases memory
};

extern Direct3D11* d3d11;

