#pragma once
#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX mat;
};

struct CB_PS_pixelshader
{
	double r_mod;
	double g_mod;
	double b_mod;
	double v_magnitude; // particle vector module (for color change) [0, 1]
};
