#include "Entity.h"

void Entity::Create(float radius, float mass, ID3D11ShaderResourceView* texture, XMFLOAT3 position, XMFLOAT2 velocity)
{
    _radius = radius;
    _mass = mass;
    _texture = texture;
    this->SetPosition(position);
    this->SetVelocity(velocity);
}

bool Entity::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader, ConstantBuffer<CB_PS_pixelshader>& cb_ps_pixelshader)
{
	_dev = device;
	_devcon = deviceContext;
	_cb_vs_vertexshader = &cb_vs_vertexshader;
    _cb_ps_pixelshader = &cb_ps_pixelshader;

    // create a square using the VERTEX struct

    Vertex v[] =
    {
        Vertex(-_radius, -_radius, 0.0f,  0.0f, 1.0f),  // Bottom Left
        Vertex(-_radius,  _radius, 0.0f,  0.0f, 0.0f),  // Top Left
        Vertex(_radius,  _radius, 0.0f,  1.0f, 0.0f),  // Top Right

        Vertex(-_radius, -_radius, 0.0f,  0.0f, 1.0f),  // Bottom Left
        Vertex(_radius,  _radius, 0.0f,  1.0f, 0.0f),  // Top Right
        Vertex(_radius, -_radius, 0.0f,  1.0f, 1.0f),  // Bottom Right
    };

    HRESULT hr;
    hr = _vertexBuffer.Initialize(_dev, v, ARRAYSIZE(v));     // create the buffer
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create vertex buffer.");
        return false;
    }

    DWORD indices[] =
    {
        0,1,2,3,4,5
    };

    hr = _indexBuffer.Initialize(_dev, indices, ARRAYSIZE(indices));
    if (FAILED(hr))
    {
        ErrorLogger::Log(hr, "Failed to create index buffer.");
        return false;
    } 

	this->UpdateWorldMatrix();
	return true;
}

void Entity::Draw(const XMMATRIX& viewProjectionMatrix)
{
    // Update constant buffers
	_cb_vs_vertexshader->_data.mat = _worldMatrix * viewProjectionMatrix;
	_cb_vs_vertexshader->_data.mat = XMMatrixTranspose(_cb_vs_vertexshader->_data.mat);
    if (!_cb_vs_vertexshader->ApplyChanges())
        return;
	_devcon->VSSetConstantBuffers(0, 1, _cb_vs_vertexshader->GetAddressOf());

    _cb_ps_pixelshader->ApplyChanges();
    _devcon->PSSetConstantBuffers(0, 1, _cb_ps_pixelshader->GetAddressOf());

	_devcon->PSSetShaderResources(0, 1, &_texture);  // set texture 
	_devcon->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	UINT offset = 0;
	_devcon->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), _vertexBuffer.StridePtr(), &offset);
	_devcon->DrawIndexed(_indexBuffer.BufferSize(), 0, 0);
}

void Entity::UpdateWorldMatrix()
{
	_worldMatrix = DirectX::XMMatrixTranslation(_pos.x, _pos.y, _pos.z);
}

void Entity::Release()
{
    if (_vertexBuffer.GetAddressOf()) _vertexBuffer.Release();
    if (_indexBuffer.GetAddressOf()) _indexBuffer.Release();
}

const XMVECTOR& Entity::GetPositionVector() const
{
    return _posVector;
}

const XMFLOAT3& Entity::GetPositionFloat3() const
{
    return _pos;
}

void Entity::SetPosition(const XMVECTOR& pos)
{
    XMStoreFloat3(&_pos, pos);
    _posVector = pos;
    this->UpdateWorldMatrix();
}

void Entity::SetPosition(const XMFLOAT3& pos)
{
    _pos = pos;
    _posVector = XMLoadFloat3(&_pos);
    this->UpdateWorldMatrix();
}

void Entity::SetPosition(float x, float y, float z)
{
    _pos = XMFLOAT3(x, y, z);
    _posVector = XMLoadFloat3(&_pos);
    this->UpdateWorldMatrix();
}

void Entity::AdjustPosition(const XMVECTOR& pos)
{
    _posVector += pos;
    XMStoreFloat3(&_pos, pos);
    this->UpdateWorldMatrix();
}

void Entity::AdjustPosition(const XMFLOAT3& pos)
{
    _pos.x += pos.x;
    _pos.y += pos.y;
    _pos.z += pos.z;
    _posVector += XMLoadFloat3(&pos);
    this->UpdateWorldMatrix();
}

void Entity::AdjustPosition(float x, float y, float z)
{
    _pos.x += x;
    _pos.y += y;
    _pos.z += z;
    _posVector += XMLoadFloat3(&_pos);
    this->UpdateWorldMatrix();
}

void Entity::AdjustPosition()
{
    this->AdjustPosition(_velocity.x, _velocity.y, 0.0f);
}

const XMFLOAT2& Entity::GetVelocityFloat2() const
{
    return _velocity;
}

void Entity::UpdateVelocity(float newXVelocity, float newYVelocity)
{
    _velocity.x += newXVelocity;
    _velocity.y += newYVelocity;
}

void Entity::SetVelocity(const XMFLOAT2& velocity)
{
    _velocity.x = velocity.x;
    _velocity.y = velocity.y;
}

void Entity::SetVelocity(float x, float y)
{
    _velocity.x = x;
    _velocity.y = y;
}

float Entity::GetRadius() const
{
    return _radius;
}

void Entity::SetRadius(float radius)
{
    _radius = radius;
}

void Entity::UpdateRadius(float radius)
{
    _radius += radius;
}

float Entity::GetMass() const
{
    return _mass;
}

void Entity::SetMass(float mass)
{
    _mass = mass;
}

void Entity::UpdateMass(float mass)
{
    _mass += mass;
}

void Entity::Attract(Entity* entity, float dt)
{
    XMFLOAT3 entityPos = entity->GetPositionFloat3();

    float xDistance = _pos.x - entityPos.x;
    float yDistance = _pos.y - entityPos.y;
    float distance = sqrtf(xDistance * xDistance + yDistance * yDistance);

    if (distance < 20.0f)
        distance = 20.0f;

    float inverseDistance = 1.0f / distance;
    float xNormalized = xDistance * inverseDistance;
    float yNormalized = yDistance * inverseDistance;

    float inverseDistanceSq = inverseDistance * inverseDistance;

    float attractionForce = -(6.673e-11) * (_mass * entity->GetMass()) * inverseDistanceSq;

    float xAccelleration = xNormalized * attractionForce * dt;
    float yAccelleration = yNormalized * attractionForce * dt;

    // Collision if the distance between objs <= sum of their radii
    // if (distance <= particle0->GetRadius() + particle1->GetRadius())

    // DEBUG
    static float max_xAcc = xAccelleration;
    if (max_xAcc < xAccelleration)
        max_xAcc = xAccelleration;

    static float min_xAcc = xAccelleration;
    if (min_xAcc > xAccelleration)
        min_xAcc = xAccelleration;

	static float max_yAcc = yAccelleration;
    if (max_yAcc < yAccelleration)
        max_yAcc = yAccelleration;

    static float min_yAcc = yAccelleration;
    if (min_yAcc > yAccelleration)
        min_yAcc = yAccelleration;

    static float max_force = attractionForce;
    if (max_force < attractionForce)
        max_force = attractionForce;

    static float min_force = attractionForce;
    if (min_force > attractionForce)
        min_force = attractionForce;

    // end DEBUG

    this->UpdateVelocity(xAccelleration, yAccelleration);
    this->AdjustPosition();
}
