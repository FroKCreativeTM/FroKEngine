#pragma once
#include "BaseCollider.h"
class BoxCollider :
    public BaseCollider
{
public:
	BoxCollider();
	virtual ~BoxCollider();

	virtual void FinalUpdate() override;
	virtual bool Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance) override;

	void SetCenter(Vec3 center) { _center = center; }

private:
	// Local ±‚¡ÿ
	Vec3		_center = Vec3(0, 0, 0);

	BoundingBox _boundingBox;
};

