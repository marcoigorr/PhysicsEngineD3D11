#pragma once
#include "ErrorLogger.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <d3dcompiler.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")
#pragma comment (lib, "DirectXTK.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "D3DCompiler.lib")
#include "ConstantBuffer.h"
#include "ConstantBufferTypes.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "Timer.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "Camera.h"
#include "Entity.h"

class Graphics
{
public:
	bool Initialize(HWND hWnd, int width, int height);
	bool InitD3D11(HWND hWnd);							// sets up and initializes Direct3D
	bool InitPipeline(void);							// loads and prepares pipeline
	bool InitGraphicsD3D11(void);						// creates the shape to render
	bool InitImGui(HWND hWnd);
	void CleanD3D(void);								// closes Direct3D and releases memory
	void RenderFrame(void);								// renders a single frame

private:
	IDXGISwapChain* _swapchain;					// pointer to swap chain interface
	ID3D11Device* _dev;							// pointer to Direct3D device interface
	ID3D11DeviceContext* _devcon;				// pointer to Direct3D device context
	ID3D11RenderTargetView* _backbuffer;		// pointer to back buffer
	ID3D11VertexShader* _pVS;					// vertex shader (run once for each vertex rendered)
	ID3D11PixelShader* _pPS;					// pixel shader (run for each pixel drawn
	ID3D11InputLayout* _pLayout;				// input layout
	ID3D11RasterizerState* _rasterizerState;

	ID3D11Texture2D* _depthStencilBuffer;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11DepthStencilState* _depthStencilState;

	ConstantBuffer<CB_VS_vertexshader> _cb_vs_vertexshader;

	std::unique_ptr<DirectX::SpriteBatch> _spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> _spriteFont;

	ID3D11SamplerState* _samplerState;
	ID3D11ShaderResourceView* _particleTexture;

	Timer _fpsTimer;

	int _wWidth;
	int _wHeight;

public:
	Camera _camera;
	Entity _entity[2];
};
