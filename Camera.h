#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class Camera
{
public:
	Camera();
	void SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ);

	const XMMATRIX& GetViewMatrix() const;
	const XMMATRIX& GetProjectionMatrix() const;

	const XMVECTOR& GetPositionVector() const;
	const XMFLOAT3& GetPositionFloat3() const;

	const XMFLOAT3& GetDefPosition() const;

	void SetPosition(const XMVECTOR& pos);
	void SetPosition(const XMFLOAT3& pos);
	void SetPosition(float x, float y, float z);
	void AdjustPosition(const XMVECTOR& pos);
	void AdjustPosition(float x, float y, float z);

	void Track(float x, float y, float z);

private:
	void UpdateViewMatrix();
	XMVECTOR _posVector;
	XMFLOAT3 _pos;
	XMVECTOR _rotVector;
	XMFLOAT3 _rot;
	XMMATRIX _viewMatrix;
	XMMATRIX _projectionMatrix;

	const XMVECTOR _DEFAULT_FORWARD_VECTOR = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	const XMVECTOR _DEFAULT_UP_VECTOR = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMFLOAT3 _defPosition;
};