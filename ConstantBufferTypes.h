#pragma once
#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX mat;
};

// Blending
struct CB_PS_pixelshader
{
	float alpha = 1.0f;
};
