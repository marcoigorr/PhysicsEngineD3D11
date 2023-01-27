#pragma once
#include <DirectXMath.h>

struct Vertex {
    Vertex() {}
    Vertex(float x, float y, float u, float v)
        : pos(x, y), texCoord(u, v) {}
    
    DirectX::XMFLOAT2 pos;
    DirectX::XMFLOAT2 texCoord; 
};
