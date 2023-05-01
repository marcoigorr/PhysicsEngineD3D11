#pragma once
#include "Entity.h"

// Axis Aligned Bounding Box
struct AABB
{
	float x, y;
	float w, h;

	bool Contains(const Entity* point)
	{
		XMFLOAT3 pos = point->GetPositionFloat3();

		return (pos.x >= this->x - this->w &&
				pos.x <= this->x + this->w &&
				pos.y >= this->y - this->h &&
				pos.y <= this->y + this->h);
	}
};

class QuadTreeNode
{
public:
	// Enumeration for the quadrants
	enum EQuadrant
	{
		NE = 0,
		NW,
		SW,
		SE,
		NONE
	};

	QuadTreeNode(const XMFLOAT2& min, const XMFLOAT2& max, QuadTreeNode* parent);
	//QuadTreeNode(AABB boundary);

	bool IsRoot() const;
	bool IsExternal() const;
	bool WasTooClose() const;

	int GetNumRenegades() const;
	int GetNum() const;

	const XMFLOAT2& GetCenterOfMass() const;
	const XMFLOAT2& GetMin() const;
	const XMFLOAT2& GetMax() const;

	double GetTheta() const;
	void SetTheta(double theta);

	EQuadrant GetQuadrant(float x, float y) const;
	QuadTreeNode* CreateQuadNode(EQuadrant eQuad);

	void Insert(Entity* newParticle, int level);
	//void Insert(Entity* point);
	//void Subdivide();

	void DrawEntities(const XMMATRIX& viewProjectionMatrix);
	void ReleaseEntities();

	//void ComputeMassDistribution();

private:
	Entity* _assignedEntity;

	float _mass;					// mass of all particles inside the node
	XMFLOAT2 _cm;					// center of Mass
	XMFLOAT2 _min;					// upper left edge of the node
	XMFLOAT2 _max;					// lower right edge of the node
	XMFLOAT2 _center;				// center of the node
	QuadTreeNode* _parent;			// the parent node
	int _num;						// the number of particles in this node
	mutable bool _bSubdivided;		// true if this node is too close to use the approximation for the force calculation

	double s_theta;
	std::vector<Entity*> s_renegades;

	QuadTreeNode* _quadNode[4];
};
