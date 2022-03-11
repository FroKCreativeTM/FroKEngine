#pragma once
#include "BaseCollider.h"

class SphereCollider : public BaseCollider
{
public:
	SphereCollider();
	virtual ~SphereCollider();

	virtual bool Collision(BaseCollider* pDst) override;
	virtual bool Collision(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance) override;
	virtual void FinalUpdate() override;

#ifdef _DEBUG
	// 디버깅용 콜리전을 그린다.
	virtual void Render() override;
#endif

public :

	void SetRadius(float radius) { _radius = radius; }
	void SetCenter(Vec3 center) { _center = center; }

	BoundingSphere GetBoundingSphere() {  return _boundingSphere;	}

private:
	// Local 기준
	float		_radius = 0.5f;
	Vec3		_center = Vec3(0, 0, 0);

	BoundingSphere _boundingSphere;
};

