#include "pch.h"
#include "CapsuleCollider.h"
#include "Physics.h"

using namespace physx;

CapsuleCollider::CapsuleCollider(physx::PxVec3 position, PxReal radius, PxReal halfHeight,
	PxMaterial& material, PxReal capsuleDensity) : BaseCollider(ColliderType::Capsule)
{
	_capsuleActor = GET_SINGLE(Physics)->GetPhysics()->createRigidDynamic(physx::PxTransform(position));

	PxTransform relativePose(PxQuat(PxHalfPi, PxVec3(0, 0, 1)));

	PxShape* aCapsuleShape = PxRigidActorExt::createExclusiveShape(*_capsuleActor,
		PxCapsuleGeometry(radius, halfHeight), material);

	aCapsuleShape->setLocalPose(relativePose);
	PxRigidBodyExt::updateMassAndInertia(*_capsuleActor, capsuleDensity);
	GET_SINGLE(Physics)->GetScene()->addActor(*_capsuleActor);
}

CapsuleCollider::~CapsuleCollider()
{
}
