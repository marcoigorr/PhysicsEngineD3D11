#include "Shader.h"

bool VertexShader::Initialize(ID3D11Device* dev, std::wstring shaderpath, D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements)
{	
	HRESULT hr;

	hr = D3DReadFileToBlob(shaderpath.c_str(), &_shaderBuffer);
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = dev->CreateVertexShader(_shaderBuffer->GetBufferPointer(), _shaderBuffer->GetBufferSize(), NULL, &_shader);
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create vertex shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = dev->CreateInputLayout(layoutDesc, numElements, _shaderBuffer->GetBufferPointer(), _shaderBuffer->GetBufferSize(), &_inputLayout);
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create input layout.");
		return false;
	}

	return true;
}

ID3D11VertexShader* VertexShader::GetShader()
{
	return _shader;
}

ID3D10Blob* VertexShader::GetBuffer()
{
	return _shaderBuffer;
}

ID3D11InputLayout* VertexShader::GetInputLayout()
{
	return _inputLayout;
}

void VertexShader::Release()
{
	_shader->Release();
	_shaderBuffer->Release();
	_inputLayout->Release();
}

bool PixelShader::Initialize(ID3D11Device* dev, std::wstring shaderpath)
{
	HRESULT hr;

	hr = D3DReadFileToBlob(shaderpath.c_str(), &_shaderBuffer);
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	hr = dev->CreatePixelShader(_shaderBuffer->GetBufferPointer(), _shaderBuffer->GetBufferSize(), NULL, &_shader);
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to create pixel shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	return true;
}

ID3D11PixelShader* PixelShader::GetShader()
{
	return _shader;
}

void PixelShader::Release()
{
	_shader->Release();
	_shaderBuffer->Release();
}

ID3D10Blob* PixelShader::GetBuffer()
{
	return _shaderBuffer;
}
