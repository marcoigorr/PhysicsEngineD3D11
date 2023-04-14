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

		return (pos.x > this->x - this->w &&
				pos.x < this->x + this->w &&
				pos.y > this->y - this->h &&
				pos.y < this->y + this->h);
	}
};

class QuadTree
{
public:
	QuadTree();
	QuadTree(AABB boundary);
	void Insert(Entity* point);
	void Subdivide();
	void Draw(const XMMATRIX& viewProjectionMatrix);
	void Release();

	void SetBoundary(AABB boundary);

private:
	AABB _boundary;
	bool _divided;
	int _capacity;
	std::vector<Entity*> _points;

	QuadTree* _northeast = nullptr;
	QuadTree* _northwest = nullptr;
	QuadTree* _southeast = nullptr;
	QuadTree* _southwest = nullptr;

};
