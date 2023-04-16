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

		_northwest->Insert(point);
		_northeast->Insert(point);
		_southwest->Insert(point);
		_southeast->Insert(point);
	}
}

void QuadTree::Subdivide()
{
	AABB NW = { _boundary.x - _boundary.w / 2, _boundary.y - _boundary.h / 2, _boundary.w / 2, _boundary.h / 2 };
	_northwest = new QuadTree(NW);

	AABB NE = { _boundary.x + _boundary.w / 2, _boundary.y - _boundary.h / 2, _boundary.w / 2, _boundary.h / 2 };
	_northeast = new QuadTree(NE);

	AABB SW = { _boundary.x - _boundary.w / 2, _boundary.y + _boundary.h / 2, _boundary.w / 2, _boundary.h / 2 };
	_southwest = new QuadTree(SW);

	AABB SE = { _boundary.x + _boundary.w / 2, _boundary.y + _boundary.h / 2, _boundary.w / 2, _boundary.h / 2 };
	_southeast = new QuadTree(SE);

	_divided = true;
}

void QuadTree::Draw(const XMMATRIX& viewProjectionMatrix)
{
	for (Entity* p : _points)
	{
		p->Draw(viewProjectionMatrix);
	}

	if (_northwest) _northwest->Draw(viewProjectionMatrix);
	if (_northeast) _northeast->Draw(viewProjectionMatrix);
	if (_southwest) _southwest->Draw(viewProjectionMatrix);
	if (_southeast) _southeast->Draw(viewProjectionMatrix);
}

void QuadTree::Release()
{
	for (Entity* p : _points)
	{
		p->Release();
	}

	if (_northwest) _northwest->Release();
	if (_northeast) _northeast->Release();
	if (_southwest) _southwest->Release();
	if (_southeast) _southeast->Release();
}

void QuadTree::SetBoundary(AABB boundary)
{
	_boundary = boundary;
}

void QuadTree::ComputeMassDistribution()
{
	// If number of particles in this node equals 1
	if (_points.size() == 1)
	{
		_centerOfMass = _points[0]->GetPositionFloat3();
		_mass = _points[0]->GetMass();
	} else {

		if (_divided)
		{
			_northwest->ComputeMassDistribution();
			_northeast->ComputeMassDistribution();
			_southwest->ComputeMassDistribution();
			_southeast->ComputeMassDistribution();
		}

	}
}


