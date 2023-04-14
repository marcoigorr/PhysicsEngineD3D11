#include "QuadTree.h"

QuadTree::QuadTree()
{
	_capacity = 1;
	_divided = false;
}

QuadTree::QuadTree(AABB boundary)
{
	_boundary = boundary;
	_capacity = 1;
	_divided = false;
}

void QuadTree::Insert(Entity* point)
{
	if (!_boundary.Contains(point))
		return;

	if (_points.size() < _capacity) 
		_points.push_back(point);
	else {
		if (!_divided)
			this->Subdivide();

		_northeast->Insert(point);
		_northwest->Insert(point);
		_southeast->Insert(point);
		_southwest->Insert(point);
	}

}

void QuadTree::Subdivide()
{
	AABB NE = { _boundary.x + _boundary.w / 2, _boundary.y - _boundary.h / 2, _boundary.w / 2, _boundary.h / 2 };
	_northeast = new QuadTree(NE);

	AABB NW = { _boundary.x - _boundary.w / 2, _boundary.y - _boundary.h / 2, _boundary.w / 2, _boundary.h / 2 };
	_northwest = new QuadTree(NW);

	AABB SE = { _boundary.x + _boundary.w / 2, _boundary.y + _boundary.h / 2, _boundary.w / 2, _boundary.h / 2 };
	_southeast = new QuadTree(SE);

	AABB SW = { _boundary.x - _boundary.w / 2, _boundary.y + _boundary.h / 2, _boundary.w / 2, _boundary.h / 2 };
	_southwest = new QuadTree(SW);

	_divided = true;
}

void QuadTree::Draw(const XMMATRIX& viewProjectionMatrix)
{
	for (Entity* p : _points)
	{
		p->Draw(viewProjectionMatrix);
	}

	if (_northeast) _northeast->Draw(viewProjectionMatrix);
	if (_northwest) _northwest->Draw(viewProjectionMatrix);
	if (_southeast) _southeast->Draw(viewProjectionMatrix);
	if (_southwest) _southwest->Draw(viewProjectionMatrix);
}

void QuadTree::Release()
{
	for (Entity* p : _points)
	{
		p->Release();
	}

	if (_northeast) _northeast->Release();
	if (_northwest) _northwest->Release();
	if (_southeast) _southeast ->Release();
	if (_southwest) _southwest->Release();
}

void QuadTree::SetBoundary(AABB boundary)
{
	_boundary = boundary;
}
