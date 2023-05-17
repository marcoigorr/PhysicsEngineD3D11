#include "QuadTree.h"

#define BIG_G 6.673e-11

QuadTreeNode::QuadTreeNode(const XMFLOAT2& min, const XMFLOAT2& max, QuadTreeNode* parent)
	:_assignedEntity()
	, _mass(0)
	, _cm()
	, _min(min)
	, _max(max)
	, _center(min.x + (max.x - min.x) / 2.0, min.y + (max.y - min.y) / 2.0)
	, _parent(parent)
	, _num(0)
	, _bSubdivided(false)
{
	_quadNode[0] = _quadNode[1] = _quadNode[2] = _quadNode[3] = nullptr;
}

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

void QuadTreeNode::Reset(const XMFLOAT2& min, const XMFLOAT2& max)
{
	if (IsRoot())
	{
		if (_assignedEntity)
			_assignedEntity = nullptr;
	}

	for (int i = 0; i < 4; i++)
	{
		delete _quadNode[i];
		_quadNode[i] = nullptr;
	}

	_min = min;
	_max = max;
	_center = XMFLOAT2(min.x + (max.x - min.x) / 2.0f, min.y + (max.y - min.y) / 2.0f);
	_num = 0;
	_mass = 0;
	_cm = XMFLOAT2(0.0f, 0.0f);

	s_renegades.clear();
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

void QuadTreeNode::ComputeMassDistribution()
{
	if (_num == 1)
	{
		XMFLOAT3 pos = _assignedEntity->GetPositionFloat3();
		_cm = XMFLOAT2(pos.x, pos.y);
		_mass = _assignedEntity->GetMass();
	}
	else
	{
		// Compute the center of mass based on the masses of all child quadrants and the center of 
		// mass as the center of mass of the child quadrants weightes with their mass
		_mass = 0.0f;
		_cm = XMFLOAT2(0.0f, 0.0f);

		for (int i = 0; i < 4; i++)
		{
			if (_quadNode[i])
			{
				_quadNode[i]->ComputeMassDistribution();
				_mass += _quadNode[i]->_mass;
				_cm.x += _quadNode[i]->_cm.x * _quadNode[i]->_mass;
				_cm.y += _quadNode[i]->_cm.y * _quadNode[i]->_mass;
			}
		}
		_cm.x /= _mass;
		_cm.y /= _mass;
	}
}

XMFLOAT2 QuadTreeNode::CalcAcc(Entity* p1, Entity* p2) const
{
	XMFLOAT2 acc(0.0f, 0.0f);

	if (p1 == p2)
	{
		return acc;
	}

	const XMFLOAT3& p1Pos(p1->GetPositionFloat3());
	const XMFLOAT3& p2Pos(p2->GetPositionFloat3());

	float xDistance = p1Pos.x - p2Pos.x;
	float yDistance = p1Pos.y - p2Pos.y;

	float r = sqrt((xDistance * xDistance) + (yDistance * yDistance) + s_soft);

	if (r > s_attractionThreshold)
	{
		float k = gamma_1 * p2->GetMass() / (r * r * r);

		acc.x = k * (p2Pos.x - p1Pos.x);
		acc.y = k * (p2Pos.y - p1Pos.y);
	}
	else
	{
		acc.x = acc.y = 0.0f;
	}

	float magnitude = sqrtf((acc.x * acc.x) + (acc.y * acc.y));
	if (acc.x >= 0.7f || acc.y >= 0.7f)
	{
		acc.x = acc.y = 0.0f;
	}


	return acc;
}

XMFLOAT2 QuadTreeNode::CalcForce(Entity* particle) const
{
	XMFLOAT2 acc = CalcTreeForce(particle);

	if (s_renegades.size())
	{
		for (std::size_t i = 0; i < s_renegades.size(); i++)
		{
			XMFLOAT2 buf = CalcAcc(particle, s_renegades[i]);
			acc.x += buf.x;
			acc.y += buf.y;
		}
	}

	return acc;
}

XMFLOAT2 QuadTreeNode::CalcTreeForce(Entity* particle) const
{
	XMFLOAT2 acc(0.0f, 0.0f);

	float r(0), d(0), k(0);
	if (_num == 1)
	{
		acc = this->CalcAcc(particle, _assignedEntity);
	}
	else
	{
		XMFLOAT3 pPos = particle->GetPositionFloat3();
		float xDistance = (pPos.x - _cm.x);
		float yDistance = (pPos.y - _cm.y);
		r = sqrt(xDistance * xDistance + yDistance * yDistance);
		d = _max.x - _min.x;
		if (d / r <= s_theta)
		{
			_bSubdivided = false;
			k = gamma_1 * _mass / (r * r * r);
			acc.x = k * (_cm.x - pPos.x);
			acc.y = k * (_cm.y - pPos.y);
		}
		else
		{
			_bSubdivided = true;

			XMFLOAT2 buf(0.0f, 0.0f);
			for (int q = 0; q < 4; q++)
			{
				if (_quadNode[q])
				{
					buf = _quadNode[q]->CalcTreeForce(particle);
					acc.x += buf.x;
					acc.y += buf.y;
				}
			}
		}
	}

	return acc;
}

void QuadTreeNode::DrawEntities(const XMMATRIX& viewProjectionMatrix)
{
	if (_assignedEntity)
		_assignedEntity->Draw(viewProjectionMatrix);

	for (int i = 0; i < 4; i++)
	{
		if (_quadNode[i])
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
	
	for (Entity* r : s_renegades)
		if (r) r->Release();
}

QuadTreeNode::~QuadTreeNode()
{
	for (int i = 0; i < 4; ++i)
		delete _quadNode[i];
}