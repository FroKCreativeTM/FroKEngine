#include "pch.h"
#include "Rigidbody.h"
#include "GameObject.h"
#include "Transform.h"
#include "Timer.h"
#include "Physics.h"

#include "BaseCollider.h"

using namespace physx;

RigidBody::RigidBody() : Component(COMPONENT_TYPE::RIGIDBODY)
{
}

RigidBody::~RigidBody()
{
}

void RigidBody::Init(BaseCollider* col)
{
	_material = PHYSICS->createMaterial(0.5f, 0.5f, 0.5f);

	_pShape = PHYSICS->createShape(PxSphereGeometry(0.5f), *_material, true);
	_pShape->setFlag(PxShapeFlag::eVISUALIZATION, true);
	_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);	// Ray, Sweep 등의 충돌을 검출할 때 사용

	// 이 아래 두 플래그는 중복되서 켜져 있으면 안 된다.
	_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

	_pActor = PHYSICS->createRigidDynamic(PxTransform(PxVec3(0.f, 2.5f, 0.f)));
	PxRigidActorExt::createExclusiveShape(*_pActor, PxBoxGeometry(2.f, 0.2f, 0.1f), *_material);
	PxRigidActorExt::createExclusiveShape(*_pActor, PxBoxGeometry(0.2f, 2.f, 0.1f), *_material);
	// Kinematic(운동역학)을 켤 것인가
	_pActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
	// 중력을 받을 것인가
	_pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	// 충돌 shape를 붙인다.
	_pActor->attachShape(*_pShape);
	// 피직스 씬에 추가한다.
	PHYSICS_SCENE->addActor(*_pActor);
}

void RigidBody::FinalUpdate()
{
}
