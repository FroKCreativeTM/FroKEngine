#include "pch.h"
#include "BoxCollider.h"
#include "GameObject.h"
#include "Transform.h"
#include "Physics.h"
#include "RigidBody.h"

using namespace physx;

BoxCollider::BoxCollider() : BaseCollider(ColliderType::Box)
{
	// Vec3 box = GetGameObject()->GetTransform()->GetLocalScale();
	// 
	// _colliderShape = PHYSICS->createShape(PxBoxGeometry(box.x, box.y, box.z),
	// 	*GetGameObject()->GetRigidBody()->GetMaterial(), true);
	// 
	// _colliderShape->setFlag(PxShapeFlag::eVISUALIZATION, true);
	// _colliderShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);	// Ray, Sweep ���� �浹�� ������ �� ���
	// 
	// // �� �Ʒ� �� �÷��״� �ߺ��Ǽ� ���� ������ �� �ȴ�.
	// _colliderShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, true);
	// _colliderShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
}

BoxCollider::~BoxCollider()
{
}

void BoxCollider::FinalUpdate()
{
	_boundingBox.Center = GetGameObject()->GetTransform()->GetWorldPosition();

	Vec3 scale = GetGameObject()->GetTransform()->GetLocalScale();

	_size = scale;
	_boundingBox.Extents = scale;
}
