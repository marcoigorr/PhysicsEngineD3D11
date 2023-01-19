#pragma once

class Direct3D11
{
public:
	IDXGISwapChain* _swapchain;             
	ID3D11Device* _dev;                     
	ID3D11DeviceContext* _devcon;
	ID3D11RenderTargetView* _backbuffer;

	void InitD3D(HWND hWnd);     // sets up and initializes Direct3D
	void CleanD3D(void);         // closes Direct3D and releases memory
	void RenderFrame(void);		 // renders a single frame
};

extern Direct3D11* d3d11;

