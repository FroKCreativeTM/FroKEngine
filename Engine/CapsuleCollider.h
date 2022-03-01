#pragma once

#include "BaseCollider.h"

enum class CAPSULE_DIRECTION : uint8
{
	X,
	Y,
	Z,
};

class CapsuleCollider : public BaseCollider
{
public:
	CapsuleCollider();
	virtual ~CapsuleCollider();

	virtual void FinalUpdate() override;

	void SetRadius(float radius) { _radius = radius; }
	void SetCenter(Vec3 center) { _center = center; }

private:
	// Local ±‚¡ÿ
	float		_radius = 0.5f;
	float		_height = 1.f;
	Vec3		_center = Vec3(0, 0, 0);

	BoundingSphere _boundingSphere;
};

