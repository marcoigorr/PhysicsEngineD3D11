#include "QuadTree.h"

QuadTreeNode::QuadTreeNode(const XMFLOAT2& min, const XMFLOAT2& max, QuadTreeNode* parent)
	:_assignedEntity()
	,_mass(0)
	,_cm()
	,_min(min)
	,_max(max)
	,_center(min.x + (max.x - min.x) / 2.0, min.y + (max.y - min.y) / 2.0)
	,_parent(parent)
	,_num(0)
	,_bSubdivided(false)
{
	_quadNode[0] = _quadNode[1] = _quadNode[2] = _quadNode[3] = nullptr;
}

//QuadTreeNode::QuadTreeNode(AABB boundary)
//{
//	_bSubdivided = false;
//}

bool QuadTreeNode::IsRoot() const
{
	return _parent == nullptr;
}

bool QuadTreeNode::IsExternal() const
{
	return _quadNode[0] == nullptr &&
		_quadNode[1] == nullptr &&
		_quadNode[2] == nullptr &&
		_quadNode[3] == nullptr;
}

bool QuadTreeNode::WasTooClose() const
{
	return _bSubdivided;
}

// Returns the number of particles not assigned to any node
int QuadTreeNode::GetNumRenegades() const
{
	return s_renegades.size();
}

// Returns the number of particles inside this node
int QuadTreeNode::GetNum() const
{
	return _num;
}

const XMFLOAT2& QuadTreeNode::GetMin() const
{
	return _min;
}

const XMFLOAT2& QuadTreeNode::GetMax() const
{
	return _max;
}

const XMFLOAT2& QuadTreeNode::GetCenterOfMass() const
{
	return _cm;
}

double QuadTreeNode::GetTheta() const
{
	return s_theta;
}

void QuadTreeNode::SetTheta(double theta)
{
	s_theta = theta;
}

QuadTreeNode::EQuadrant QuadTreeNode::GetQuadrant(float x, float y) const
{
	if (x <= _center.x && y <= _center.y)
	{
		return SW;
	}
	else if (x <= _center.x && y >= _center.y)
	{
		return NW;
	}
	else if (x >= _center.x && y >= _center.y)
	{
		return NE;
	}
	else if (x >= _center.x && y <= _center.y)
	{
		return SE;
	}
	else if (x > _max.x || y > _max.y || x < _min.x || y < _min.y)
	{
		return NONE;
	}
	else
	{
		return NONE;
	}
}

QuadTreeNode* QuadTreeNode::CreateQuadNode(EQuadrant eQuad)
{
	if (eQuad == SE)	
		return new QuadTreeNode(_center, _max, this);

	else if (eQuad == SW)
		return new QuadTreeNode(XMFLOAT2(_min.x, _center.y),
								XMFLOAT2(_center.x, _max.y),
								this);
	else if (eQuad == NW)
		return new QuadTreeNode(_min, _center, this);

	else if (eQuad == NE)
		return new QuadTreeNode(XMFLOAT2(_center.x, _min.y),
								XMFLOAT2(_max.x, _center.y),
								this);
}

void QuadTreeNode::Insert(Entity* newParticle, int level)
{
	// Check if Entity is inside bounding box
	const XMFLOAT3 pos = newParticle->GetPositionFloat3();
	if ((pos.x < _min.x || pos.x > _max.x) || (pos.y > _min.y || pos.y < _max.y))
	{
		return;
	}

	if (_num > 1)
	{
		EQuadrant eQuad = GetQuadrant(pos.x, pos.y);
		if (!_quadNode[eQuad])
			_quadNode[eQuad] = CreateQuadNode(eQuad);

		_quadNode[eQuad]->Insert(newParticle, level + 1);
	}
	else if (_num == 1)
	{
		const XMFLOAT3 pos2 = _assignedEntity->GetPositionFloat3();

		// There are two bodies at the exact same coordinates, put it in the renegade vector.
		if (pos.x == pos2.x && pos.y == pos2.y)
		{
			s_renegades.push_back(newParticle);	
		}
		else
		{
			// There is already a particle, subdivide the node and relocate that particle
			EQuadrant eQuad = GetQuadrant(pos2.x, pos2.y);
			if (_quadNode[eQuad] == nullptr)
				_quadNode[eQuad] = CreateQuadNode(eQuad);
			_quadNode[eQuad]->Insert(_assignedEntity, level + 1);

			eQuad = GetQuadrant(pos.x, pos.y);
			if (!_quadNode[eQuad])
				_quadNode[eQuad] = CreateQuadNode(eQuad);
			_quadNode[eQuad]->Insert(newParticle, level + 1);
		}
	}
	else if (_num == 0)
	{
		_assignedEntity = newParticle;
	}

	_num++;
}

//void QuadTreeNode::Insert(Entity* point)
//{
//	if (!_boundingBox.Contains(point))
//		return;
//
//	if (_points.size() < 1) 
//		_points.push_back(point);
//	else {
//		if (!_divided)
//			this->Subdivide();
//
//		_northwest->Insert(point);
//		_northeast->Insert(point);
//		_southwest->Insert(point);
//		_southeast->Insert(point);
//	}
//}

//void QuadTreeNode::Subdivide()
//{
//	AABB NW = { _boundingBox.x - _boundingBox.w / 2, _boundingBox.y - _boundingBox.h / 2, _boundingBox.w / 2, _boundingBox.h / 2 };
//	_northwest = new QuadTreeNode(NW);
//
//	AABB NE = { _boundingBox.x + _boundingBox.w / 2, _boundingBox.y - _boundingBox.h / 2, _boundingBox.w / 2, _boundingBox.h / 2 };
//	_northeast = new QuadTreeNode(NE);
//
//	AABB SW = { _boundingBox.x - _boundingBox.w / 2, _boundingBox.y + _boundingBox.h / 2, _boundingBox.w / 2, _boundingBox.h / 2 };
//	_southwest = new QuadTreeNode(SW);
//
//	AABB SE = { _boundingBox.x + _boundingBox.w / 2, _boundingBox.y + _boundingBox.h / 2, _boundingBox.w / 2, _boundingBox.h / 2 };
//	_southeast = new QuadTreeNode(SE);
//
//	_divided = true;
//}

void QuadTreeNode::DrawEntities(const XMMATRIX& viewProjectionMatrix)
{
	if (_parent == nullptr)
		_assignedEntity->Draw(viewProjectionMatrix);

	if (_assignedEntity != nullptr)
		_assignedEntity->Draw(viewProjectionMatrix);

	for (int i = 0; i < 4; i++)
	{
		if (_quadNode[i] != nullptr)
			_quadNode[i]->DrawEntities(viewProjectionMatrix);
	}
}

void QuadTreeNode::ReleaseEntities()
{
	if (_parent == nullptr)
		_assignedEntity->Release();

	if (_assignedEntity != nullptr)
		_assignedEntity->Release();

	for (int i = 0; i < 4; i++)
	{
		if (_quadNode[i] != nullptr)
			_quadNode[i]->ReleaseEntities();
	}
}

//void QuadTreeNode::ComputeMassDistribution()
//{
//	
//}


