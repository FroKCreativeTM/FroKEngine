#pragma once
#include "BaseCollider.h"
class UICollider :
    public BaseCollider
{
public:
	UICollider();
	virtual ~UICollider();

	virtual bool Collision(int32 x, int32 y) override;
	virtual void FinalUpdate() override;

public:
	void SetCenter(Vec3 center) { _center = center; }
private:
	// Local ±‚¡ÿ
	Vec3		_center = Vec3(0, 0, 0);
};

