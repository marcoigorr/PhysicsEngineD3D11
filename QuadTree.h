#pragma once
#include "Graphics.h"

class QuadTree
{
public:
	void Insert(Entity* point);
	void Subdivide();

private:
	XMFLOAT2 _boundary;
	int _capacity;
	std::vector<Entity*> _points;

};
