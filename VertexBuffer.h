#ifndef VertexBuffer_h__
#define VertexBuffer_h__
#include <d3d11.h>
#include <memory>

template<class T>
class VertexBuffer
{
private:
	VertexBuffer(const VertexBuffer<T>& rhs);

private:
	ID3D11Buffer* _buffer;
	std::unique_ptr<UINT> _stride;
	UINT _bufferSize = 0;

public:
	VertexBuffer() {}

	ID3D11Buffer* Get()const
	{
		return _buffer;
	}

	ID3D11Buffer* const* GetAddressOf()const
	{
		return &_buffer;
	}

	UINT BufferSize() const
	{
		return _bufferSize;
	}

	const UINT Stride() const
	{
		return *_stride.get();
	}

	const UINT* StridePtr() const
	{
		return _stride.get();
	}

	HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* devcon, T* data, UINT numVertices)
	{
		_bufferSize = numVertices;
		_stride = std::make_unique<UINT>(sizeof(T));

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.ByteWidth = sizeof(T) * numVertices;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = data;

		HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &_buffer);
		return hr;
	}

	void Release()
	{
		_buffer->Release();
	}
};

#endif // VertexBuffer_h__