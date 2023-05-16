#pragma once
#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX mat;
};

struct CB_PS_pixelshader
{
	double v_magnitude; // particle vector module (for color change) [0, 1]
};
