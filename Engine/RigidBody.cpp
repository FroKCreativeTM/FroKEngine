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
	_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);	// Ray, Sweep ���� �浹�� ������ �� ���

	// �� �Ʒ� �� �÷��״� �ߺ��Ǽ� ���� ������ �� �ȴ�.
	_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);

	_pActor = PHYSICS->createRigidDynamic(PxTransform(PxVec3(0.f, 2.5f, 0.f)));
	PxRigidActorExt::createExclusiveShape(*_pActor, PxBoxGeometry(2.f, 0.2f, 0.1f), *_material);
	PxRigidActorExt::createExclusiveShape(*_pActor, PxBoxGeometry(0.2f, 2.f, 0.1f), *_material);
	// Kinematic(�����)�� �� ���ΰ�
	_pActor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, false);
	// �߷��� ���� ���ΰ�
	_pActor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);
	// �浹 shape�� ���δ�.
	_pActor->attachShape(*_pShape);
	// ������ ���� �߰��Ѵ�.
	PHYSICS_SCENE->addActor(*_pActor);
}

void RigidBody::FinalUpdate()
{
}
