#ifndef IndicesBuffer_h__
#define IndicesBuffer_h__
#include <d3d11.h>
#include <vector>

class IndexBuffer
{
private: 
	IndexBuffer(const IndexBuffer& rhs);

private:
	ID3D11Buffer* _buffer;
	UINT _bufferSize = 0;

public:
	IndexBuffer() {}

	ID3D11Buffer* Get()const
	{
		return _buffer;
	}

	ID3D11Buffer* const* GetAddressOf()const
	{
		return &_buffer;
	}

	UINT BufferSize()const
	{
		return _bufferSize;
	}

	HRESULT Initialize(ID3D11Device* dev, DWORD* data, UINT numIndices)
	{
		_bufferSize = numIndices;

		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

		indexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		indexBufferDesc.ByteWidth = sizeof(DWORD) * numIndices;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		ZeroMemory(&indexBufferData, sizeof(indexBufferData));

		indexBufferData.pSysMem = data;

		HRESULT hr = dev->CreateBuffer(&indexBufferDesc, &indexBufferData, &_buffer);
		return hr;
	}

	void Release()
	{
		_buffer->Release();
	}
};

#endif // !IndicesBuffer_h__

