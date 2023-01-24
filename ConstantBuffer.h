#ifndef ConstantBuffer_h__
#define ConstantBuffer_h__
#include <d3d11.h>
#include "ErrorLogger.h"

template<class T>
class ConstantBuffer
{
private:
	ConstantBuffer(const ConstantBuffer<T>& rhs);

private:
	ID3D11Buffer* _buffer;
	ID3D11DeviceContext* _devcon = nullptr;

public:
	ConstantBuffer() {}

	T _data;

	ID3D11Buffer* Get()const
	{
		return _buffer;
	}

	ID3D11Buffer* const* GetAddressOf()const
	{
		return &_buffer;
	}

	HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		_devcon = deviceContext;

		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16)));
		bufferDesc.StructureByteStride = 0;

		HRESULT hr = device->CreateBuffer(&bufferDesc, 0, &_buffer);
		return hr;
	}

	void Release()
	{
		_buffer->Release();
	}

	bool ApplyChanges()
	{
		D3D11_MAPPED_SUBRESOURCE mp;
		HRESULT hr = _devcon->Map(_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mp);  // mapping a buffer allow us to access it (GPU stops working on it)
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to map constant buffer.");
			return false;
		}
		memcpy(mp.pData, &_data, sizeof(T));
		_devcon->Unmap(_buffer, NULL);
	}
};

#endif // !ConstantBuffer_h__
