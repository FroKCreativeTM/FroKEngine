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
	_transform.p = PxVec3();
	_transform.q = PxQuat();

	auto col = GetGameObject()->GetCollider();

	// 콜리전이 있다면
	if (col)
	{
		_pActor = PHYSICS->createRigidDynamic(_transform);
		// Kinematic(운동역학)을 켤 것인가
		_pActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
		// 중력을 받을 것인가
		_pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		// 충돌 shape를 붙인다.
		_pActor->attachShape(*col->GetShape());
		// 피직스 씬에 추가한다.
		PHYSICS_SCENE->addActor(*_pActor);
	}
}

RigidBody::~RigidBody()
{
}

void RigidBody::FinalUpdate()
{

}
