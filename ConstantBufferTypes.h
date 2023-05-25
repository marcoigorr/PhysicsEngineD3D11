#pragma once
#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX mat;
};

struct CB_PS_pixelshader
{
	float v_magnitude; // particle vector module (for color change) [0, 1]
	float r_mod;
	float g_mod;
	float b_mod;
	bool enableColor;
};
