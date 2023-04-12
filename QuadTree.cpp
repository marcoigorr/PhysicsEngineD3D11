#include "QuadTree.h"

void QuadTree::Insert(Entity* point)
{
	if (_points.size() < _capacity)
		_points.push_back(point);
	else
		this->Subdivide();
}

void QuadTree::Subdivide()
{
	
}
