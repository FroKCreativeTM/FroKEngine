#pragma once
#include "BaseCollider.h"
class BoxCollider :
    public BaseCollider
{
public:
	BoxCollider();
	virtual ~BoxCollider();

	virtual void FinalUpdate() override;

	void SetCenter(Vec3 center) { _center = center; }

	BoundingBox GetBoundingBox() { return _boundingBox; }

private:
	// Local ±‚¡ÿ
	Vec3		_center = Vec3(0, 0, 0);
	Vec3		_size = Vec3(1.f, 1.f, 1.f);

	BoundingBox _boundingBox;
	physx::PxShape*				_boxShape;
};

