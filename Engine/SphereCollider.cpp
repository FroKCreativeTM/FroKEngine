#include "pch.h"
#include "SphereCollider.h"

#include "GameObject.h"
#include "Transform.h"

#include "BoxCollider.h"
#include "FrustumCollider.h"
#include "OrientBoxCollider.h"


SphereCollider::SphereCollider() : 
	BaseCollider(ColliderType::Sphere)
{

}

SphereCollider::~SphereCollider()
{

}

bool SphereCollider::Collision(BaseCollider* pDst)
{
	switch (pDst->GetColliderType())
	{
	case ColliderType::Sphere:
		return CollisionSphereToSphere(GetBoundingSphere(), ((SphereCollider*)pDst)->GetBoundingSphere());
	case ColliderType::Box:
		return CollisionBoxToSphere(((BoxCollider*)pDst)->GetBoundingBox(), GetBoundingSphere());
	case ColliderType::OBB:
		return CollisionSphereToOrientedBox(GetBoundingSphere(), ((OrientBoxCollider*)pDst)->GetBoundingOBB());
	case ColliderType::Frustum:
		return CollisionSphereToFrustum(GetBoundingSphere(), ((FrustumCollider*)pDst)->GetBoundingFrustum());
	}

	return false;
}

bool SphereCollider::Collision(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return CollisionSphereToRay(GetBoundingSphere(), rayOrigin, rayDir, distance);
}

void SphereCollider::FinalUpdate()
{
	_boundingSphere.Center = GetGameObject()->GetTransform()->GetWorldPosition();

	Vec3 scale = GetGameObject()->GetTransform()->GetLocalScale();
	_boundingSphere.Radius = _radius * max(max(scale.x, scale.y), scale.z);
}

void SphereCollider::Render()
{
}
