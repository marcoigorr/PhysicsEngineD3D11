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

	const XMVECTOR& GetPositionVector() const;
	const XMFLOAT3& GetPositionFloat3() const;

	void SetPosition(const XMVECTOR& pos);
	void SetPosition(const XMFLOAT3& pos);
	void SetPosition(float x, float y, float z);
	void AdjustPosition(const XMVECTOR& pos);
	void AdjustPosition(const XMFLOAT3& pos);
	void AdjustPosition(float x, float y, float z);

private:
	void UpdateWorldMatrix();

	ID3D11Device* _dev = nullptr;
	ID3D11DeviceContext* _devcon = nullptr;
	ConstantBuffer<CB_VS_vertexshader>* _cb_vs_vertexshader = nullptr;
	ID3D11ShaderResourceView* _texture = nullptr;

	VertexBuffer<Vertex> _vertexBuffer;
	IndexBuffer _indexBuffer;

	XMMATRIX _worldMatrix = XMMatrixIdentity();

	XMVECTOR _posVector;
	XMFLOAT3 _pos;

	const XMVECTOR _DEFAULT_FORWARD_VECTOR =	XMVectorSet( 0.0f, 0.0f,  1.0f, 0.0f);
	const XMVECTOR _DEFAULT_UP_VECTOR =			XMVectorSet( 0.0f, 1.0f,  0.0f, 0.0f);
	const XMVECTOR _DEFAULT_BACKWARD_VECTOR =	XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f);
	const XMVECTOR _DEFAULT_LEFT_VECTOR =		XMVectorSet(-1.0f, 0.0f,  0.0f, 0.0f);
	const XMVECTOR _DEFAULT_RIGHT_VECTOR =		XMVectorSet( 1.0f, 0.0f,  0.0f, 0.0f);
};