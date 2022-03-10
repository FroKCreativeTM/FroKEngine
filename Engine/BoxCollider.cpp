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
	// _colliderShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, true);	// Ray, Sweep 등의 충돌을 검출할 때 사용
	// 
	// // 이 아래 두 플래그는 중복되서 켜져 있으면 안 된다.
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
