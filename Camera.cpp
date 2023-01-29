#include "Camera.h"

Camera::Camera()
{
	this->_pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->_posVector = XMLoadFloat3(&this->_pos);
	this->_rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->_rotVector = XMLoadFloat3(&this->_rot);
	this->UpdateViewMatrix();
}

void Camera::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
	this->_projectionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

const XMMATRIX& Camera::GetViewMatrix() const
{
	return this->_viewMatrix;
}

const XMMATRIX& Camera::GetProjectionMatrix() const
{
	return this->_projectionMatrix;
}

const XMVECTOR& Camera::GetPositionVector() const
{
	return this->_posVector;
}

const XMFLOAT3& Camera::GetPositionFloat3() const
{
	return this->_pos;
}

const XMVECTOR& Camera::GetRotationVector() const
{
	return this->_rotVector;
}

const XMFLOAT3& Camera::GetRotationFloat3() const
{
	return this->_rot;
}

void Camera::SetPosition(const XMVECTOR& pos)
{
	XMStoreFloat3(&this->_pos, pos);
	this->_posVector = pos;
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
	this->_pos = XMFLOAT3(x, y, z);
	this->_posVector = XMLoadFloat3(&this->_pos);
	this->UpdateViewMatrix();
}

void Camera::AdjustPosition(const XMVECTOR& pos)
{
	this->_posVector += pos;
	XMStoreFloat3(&this->_pos, this->_posVector);
	this->UpdateViewMatrix();
}

void Camera::AdjustPosition(float x, float y, float z)
{
	this->_pos.x += x;
	this->_pos.y += y;
	this->_pos.z += z;
	this->_posVector = XMLoadFloat3(&this->_pos);
	this->UpdateViewMatrix();
}

void Camera::SetRotation(const XMVECTOR& rot)
{
	this->_rotVector = rot;
	XMStoreFloat3(&this->_rot, rot);
	this->UpdateViewMatrix();
}

void Camera::SetRotation(float x, float y, float z)
{
	this->_rot = XMFLOAT3(x, y, z);
	this->_rotVector = XMLoadFloat3(&this->_rot);
	this->UpdateViewMatrix();
}

void Camera::AdjustRotation(const XMVECTOR& rot)
{
	this->_rotVector += rot;
	XMStoreFloat3(&this->_rot, this->_rotVector);
	this->UpdateViewMatrix();
}

void Camera::AdjustRotation(float x, float y, float z)
{
	this->_rot.x += x;
	this->_rot.y += y;
	this->_rot.z += z;
	this->_rotVector = XMLoadFloat3(&this->_rot);
	this->UpdateViewMatrix();
}

void Camera::UpdateViewMatrix() //Updates view matrix and also updates the movement vectors
{
	//Calculate camera rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(this->_rot.x, this->_rot.y, this->_rot.z);
	//Calculate unit vector of cam target based off camera forward value transformed by cam rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(this->_DEFAULT_FORWARD_VECTOR, camRotationMatrix);
	//Adjust cam target to be offset by the camera's current position
	camTarget += this->_posVector;
	//Calculate up direction based on current rotation
	XMVECTOR upDir = XMVector3TransformCoord(this->_DEFAULT_UP_VECTOR, camRotationMatrix);
	//Rebuild view matrix
	this->_viewMatrix = XMMatrixLookAtLH(this->_posVector, camTarget, upDir);
}