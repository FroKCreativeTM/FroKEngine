#include "pch.h"
#include "BoxCollider.h"
#include "GameObject.h"
#include "Transform.h"
#include "Physics.h"
#include "RigidBody.h"

#include "SphereCollider.h"
#include "FrustumCollider.h"
#include "OrientBoxCollider.h"

BoxCollider::BoxCollider() : BaseCollider(ColliderType::Box)
{
}

BoxCollider::~BoxCollider()
{
}

void BoxCollider::Render()
{
}

bool BoxCollider::Collision(BaseCollider* pDst)
{
	switch (pDst->GetColliderType())
	{
	case ColliderType::Sphere:
		return CollisionBoxToSphere(GetBoundingBox(), ((SphereCollider*)pDst)->GetBoundingSphere());
	case ColliderType::Box:
		return CollisionBoxToBox(GetBoundingBox(), ((BoxCollider*)pDst)->GetBoundingBox());
	case ColliderType::OBB:
		return CollisionBoxToOrientedBox(GetBoundingBox(), ((OrientBoxCollider*)pDst)->GetBoundingOBB());
	case ColliderType::Frustum:
		return CollisionBoxToFrustum(GetBoundingBox(), ((FrustumCollider*)pDst)->GetBoundingFrustum());
	}

	return false;
}

bool BoxCollider::Collision(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return CollisionBoxToRay(GetBoundingBox(), rayOrigin, rayDir, distance);
}

void BoxCollider::FinalUpdate()
{
	_boundingBox.Center = GetGameObject()->GetTransform()->GetWorldPosition();

	Vec3 scale = GetGameObject()->GetTransform()->GetLocalScale();

	_size = scale;
	_boundingBox.Extents = scale;
}
