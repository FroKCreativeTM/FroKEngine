#pragma once
#include "BaseCollider.h"
class BoxCollider :
    public BaseCollider
{
public:
	BoxCollider();
	virtual ~BoxCollider();

	virtual bool Collision(BaseCollider* pDst) override;
	virtual bool Collision(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance) override;
	virtual void FinalUpdate() override;

#ifdef _DEBUG
	// 디버깅용 콜리전을 그린다.
	virtual void Render() override;
#endif

public : 

	void SetCenter(Vec3 center) { _center = center; }

	BoundingBox GetBoundingBox() { return _boundingBox; }

private:
	// Local 기준
	Vec3		_center = Vec3(0, 0, 0);
	Vec3		_size = Vec3(1.f, 1.f, 1.f);

	BoundingBox _boundingBox;
};

