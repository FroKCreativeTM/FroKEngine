#pragma once

#include "BaseCollider.h"


class CapsuleCollider : public BaseCollider
{
public : 
	CapsuleCollider(physx::PxVec3 position, physx::PxReal radius, physx::PxReal halfHeight,
		physx::PxMaterial& material, physx::PxReal capsuleDensity);
	virtual ~CapsuleCollider();

	virtual void FinalUpdate() override;

private:
	physx::PxRigidDynamic*		_capsuleActor;
};

