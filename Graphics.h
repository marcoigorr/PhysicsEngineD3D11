#pragma once
#include "ErrorLogger.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")
#pragma comment (lib, "D3DCompiler.lib")
#include "Vertex.h"

class Graphics
{
public:
	bool InitD3D(HWND hWnd, int width, int height);     // sets up and initializes Direct3D
	void CleanD3D(void);         // closes Direct3D and releases memory
	void RenderFrame(void);		 // renders a single frame
	bool InitPipeline(void);	 // loads and prepares pipeline
	bool InitGraphicsD3D11(void);

private:
	IDXGISwapChain* _swapchain;
	ID3D11Device* _dev;
	ID3D11DeviceContext* _devcon;
	ID3D11RenderTargetView* _backbuffer;
	ID3D11VertexShader* _pVS;	// vertex shader (run once for each vertex rendered)
	ID3D11PixelShader* _pPS;	// pixel shader (run for each pixel drawn
	ID3D11Buffer* _pVBuffer;	// vertex buffer
	ID3D11InputLayout* _pLayout; // input layout
};
