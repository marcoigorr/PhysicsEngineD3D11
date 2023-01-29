#pragma once
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "ConstantBufferTypes.h"

using namespace DirectX;

class Entity
{
public:
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexbuffer);
	void SetTexture(ID3D11ShaderResourceView* texture);
	void Draw(const XMMATRIX& viewProjectionMatrix);
	void Release();

private:
	void UpdateWorldMatrix();

	ID3D11Device* _dev = nullptr;
	ID3D11DeviceContext* _devcon = nullptr;
	ConstantBuffer<CB_VS_vertexshader>* _cb_vs_vertexshader = nullptr;
	ID3D11ShaderResourceView* _texture = nullptr;

	VertexBuffer<Vertex> _vertexBuffer;
	IndexBuffer _indexBuffer;

	XMMATRIX _worldMatrix = XMMatrixIdentity();
};