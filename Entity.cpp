#include "Entity.h"

bool Entity::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexbuffer)
{
	_dev = device;
	_devcon = deviceContext;
	_texture = texture;
	_cb_vs_vertexshader = &cb_vs_vertexbuffer;

    // create a square using the VERTEX struct
    Vertex v[] =
    {
        Vertex(-0.5f, -0.5f, 0.0f,  0.0f, 1.0f),  // Bottom Left
        Vertex(-0.5f,  0.5f, 0.0f,  0.0f, 0.0f),  // Top Left
        Vertex(0.5f,  0.5f, 0.0f,  1.0f, 0.0f),  // Top Right

        Vertex(-0.5f, -0.5f, 0.0f,  0.0f, 1.0f),  // Bottom Left
        Vertex(0.5f,  0.5f, 0.0f,  1.0f, 0.0f),  // Top Right
        Vertex(0.5f, -0.5f, 0.0f,  1.0f, 1.0f),  // Bottom Right
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

void Entity::SetTexture(ID3D11ShaderResourceView* texture)
{
	_texture = texture;
}

void Entity::Draw(const XMMATRIX& viewProjectionMatrix)
{
	_cb_vs_vertexshader->_data.mat = _worldMatrix * viewProjectionMatrix;
	_cb_vs_vertexshader->_data.mat = XMMatrixTranspose(_cb_vs_vertexshader->_data.mat);
	_cb_vs_vertexshader->ApplyChanges();
	_devcon->VSSetConstantBuffers(0, 1, _cb_vs_vertexshader->GetAddressOf());

	_devcon->PSSetShaderResources(0, 1, &_texture);  // set texture 
	_devcon->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	UINT offset = 0;
	_devcon->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), _vertexBuffer.StridePtr(), &offset);
	_devcon->DrawIndexed(_indexBuffer.BufferSize(), 0, 0);
}

void Entity::UpdateWorldMatrix()
{
	_worldMatrix = XMMatrixIdentity();
}

void Entity::Release()
{
    if (_vertexBuffer.GetAddressOf()) _vertexBuffer.Release();
    if (_indexBuffer.GetAddressOf()) _indexBuffer.Release();
}