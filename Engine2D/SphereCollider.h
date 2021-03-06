#pragma once
#include "BaseCollider.h"

class SphereCollider : public BaseCollider
{
public:
	SphereCollider();
	virtual ~SphereCollider();

	virtual bool Collision(BaseCollider* pDst) override;
	virtual void FinalUpdate() override;

public :

	void SetRadius(float radius) { _radius = radius; }
	void SetCenter(Vec3 center) { _center = center; }

	BoundingSphere GetBoundingSphere() {  return _boundingSphere;	}

private:
	// Local ????
	float		_radius = 0.5f;
	Vec3		_center = Vec3(0, 0, 0);

	BoundingSphere _boundingSphere;
	physx::PxSphereGeometry _sphere;
};

