#include "Camera.h"

Camera::Camera()
{
	_pos = XMFLOAT3(0.0f, 0.0f, -50.0f);
	_posVector = XMLoadFloat3(&_pos);
	_rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_rotVector = XMLoadFloat3(&_rot);
	this->UpdateViewMatrix();
}

void Camera::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
	_projectionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

const XMMATRIX& Camera::GetViewMatrix() const
{
	return _viewMatrix;
}

const XMMATRIX& Camera::GetProjectionMatrix() const
{
	return _projectionMatrix;
}

const XMVECTOR& Camera::GetPositionVector() const
{
	return _posVector;
}

const XMFLOAT3& Camera::GetPositionFloat3() const
{
	return _pos;
}

void Camera::SetPosition(const XMVECTOR& pos)
{
	XMStoreFloat3(&_pos, pos);
	_posVector = pos;
	this->UpdateViewMatrix();
}

void Camera::SetPosition(const XMFLOAT3& pos)
{
	_pos = pos;
	_posVector = XMLoadFloat3(&_pos);
	this->UpdateViewMatrix();
}

void Camera::SetPosition(float x, float y, float z)
{
	_pos = XMFLOAT3(x, y, z);
	_posVector = XMLoadFloat3(&_pos);
	this->UpdateViewMatrix();
}

void Camera::AdjustPosition(const XMVECTOR& pos)
{
	_posVector += pos;
	XMStoreFloat3(&_pos, _posVector);
	this->UpdateViewMatrix();
}

void Camera::AdjustPosition(float x, float y, float z)
{
	_pos.x += x;
	_pos.y += y;
	_pos.z += z;
	_posVector = XMLoadFloat3(&_pos);
	this->UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	//Calculate camera rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(_rot.x, _rot.y, _rot.z);
	//Calculate unit vector of cam target based off camera forward value transformed by cam rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(_DEFAULT_FORWARD_VECTOR, camRotationMatrix);
	//Adjust cam target to be offset by the camera's current position
	camTarget += _posVector;
	//Calculate up direction based on current rotation
	XMVECTOR upDir = XMVector3TransformCoord(_DEFAULT_UP_VECTOR, camRotationMatrix);
	//Rebuild view matrix
	_viewMatrix = XMMatrixLookAtLH(_posVector, camTarget, upDir);
}