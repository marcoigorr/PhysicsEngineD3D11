#pragma once
#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "ConstantBufferTypes.h"
#include <array>

using namespace DirectX;

class Entity
{
public:
	void Create(float radius, float mass, ID3D11ShaderResourceView* texture, XMFLOAT3 position, XMFLOAT2 velocity);
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_pixelshader>& cb_ps_pixelshader);
	void Draw(const XMMATRIX& viewProjectionMatrix);
	void Release();

	void UpdateVelocity(float newXVelocity, float newYVelocity);

	const XMVECTOR& GetPositionVector() const;
	const XMFLOAT3& GetPositionFloat3() const;

 	void SetPosition(const XMVECTOR& pos);
	void SetPosition(const XMFLOAT3& pos);
	void SetPosition(float x, float y, float z);
	void AdjustPosition(const XMVECTOR& pos);
	void AdjustPosition(const XMFLOAT3& pos);
	void AdjustPosition(float x, float y, float z);
	void AdjustPosition();

	const XMFLOAT2& GetVelocityFloat2() const;

	void SetVelocity(const XMFLOAT2& velocity);
	void SetVelocity(float x, float y);

	float GetRadius() const;

	void SetRadius(float radius);
	void UpdateRadius(float radius);

	float GetMass() const;

	void SetMass(float mass);
	void UpdateMass(float mass);

	XMFLOAT2 CalcAttractionAcc(Entity* entity);

private:
	void UpdateWorldMatrix();

	ID3D11Device* _dev = nullptr;
	ID3D11DeviceContext* _devcon = nullptr;
	ConstantBuffer<CB_VS_vertexshader>* _cb_vs_vertexshader = nullptr;
	ConstantBuffer<CB_PS_pixelshader>* _cb_ps_pixelshader = nullptr;
	ID3D11ShaderResourceView* _texture = nullptr;

	VertexBuffer<Vertex> _vertexBuffer;
	IndexBuffer _indexBuffer;

	XMMATRIX _worldMatrix = XMMatrixIdentity();

	XMVECTOR _posVector;
	XMFLOAT3 _pos;

	XMFLOAT2 _velocity;

	float _radius;
	float _mass;

	const XMVECTOR _DEFAULT_FORWARD_VECTOR =	XMVectorSet( 0.0f, 0.0f,  1.0f, 0.0f);
	const XMVECTOR _DEFAULT_UP_VECTOR =			XMVectorSet( 0.0f, 1.0f,  0.0f, 0.0f);
	const XMVECTOR _DEFAULT_BACKWARD_VECTOR =	XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f);
	const XMVECTOR _DEFAULT_LEFT_VECTOR =		XMVectorSet(-1.0f, 0.0f,  0.0f, 0.0f);
	const XMVECTOR _DEFAULT_RIGHT_VECTOR =		XMVectorSet( 1.0f, 0.0f,  0.0f, 0.0f);
};