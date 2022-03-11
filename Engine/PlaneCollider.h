#pragma once

#include "BaseCollider.h"

class PlaneCollider : public BaseCollider
{
public:
	PlaneCollider();
	virtual ~PlaneCollider();

	virtual bool Collision(BaseCollider* pDst) override;
	virtual bool Collision(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance) override;

	virtual void FinalUpdate() override;


#ifdef _DEBUG
	// ������ �ݸ����� �׸���.
	virtual void Render() override;
#endif

public:
	Vec3  GetPlaneNormal() { return _normalVec; }

private:
	Vec3		_normalVec;
};

