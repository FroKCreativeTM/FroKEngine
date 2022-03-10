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

	// �ݸ����� �ִٸ�
	if (col)
	{
		_pActor = PHYSICS->createRigidDynamic(_transform);
		// Kinematic(�����)�� �� ���ΰ�
		_pActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
		// �߷��� ���� ���ΰ�
		_pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
		// �浹 shape�� ���δ�.
		_pActor->attachShape(*col->GetShape());
		// ������ ���� �߰��Ѵ�.
		PHYSICS_SCENE->addActor(*_pActor);
	}
}

RigidBody::~RigidBody()
{
}

void RigidBody::FinalUpdate()
{

}
