#pragma once
#include "ErrorLogger.h"
#pragma comment(lib, "D3DCompiler.lib")
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

class VertexShader
{
public:
	bool Initialize(ID3D11Device* dev, std::wstring shaderpath, D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements);
	ID3D11VertexShader* GetShader();
	ID3D10Blob* GetBuffer();
	ID3D11InputLayout* GetInputLayout();
	void Release();

private:
	ID3D11VertexShader* _shader;
	ID3D10Blob* _shaderBuffer;
	ID3D11InputLayout* _inputLayout;
};

class PixelShader
{
public:
	bool Initialize(ID3D11Device* dev, std::wstring shaderpath);
	ID3D11PixelShader* GetShader();
	ID3D10Blob* GetBuffer();
	void Release();

private:
	ID3D11PixelShader* _shader;
	ID3D10Blob* _shaderBuffer;
};
