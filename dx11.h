#pragma once

class Direct3D11
{
public:
	IDXGISwapChain* _swapchain;             
	ID3D11Device* _dev;                     
	ID3D11DeviceContext* _devcon;
	ID3D11RenderTargetView* _backbuffer;
	ID3D11VertexShader* _pVS;	// vertex shader (run once for each vertex rendered)
	ID3D11PixelShader* _pPS;	// pixel shader (run for each pixel drawn
	ID3D11Buffer* _pVBuffer;	// vertex buffer
	ID3D11InputLayout* _pLayout; // input layout

	void InitD3D(HWND hWnd);     // sets up and initializes Direct3D
	void CleanD3D(void);         // closes Direct3D and releases memory
	void RenderFrame(void);		 // renders a single frame
	void InitPipeline(void);	 // loads and prepares pipeline
	void InitGraphics(void);
};

extern Direct3D11* d3d11;

