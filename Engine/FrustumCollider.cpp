#include "pch.h"
#include "FrustumCollider.h"

#include "SphereCollider.h"
#include "BoxCollider.h"
#include "OrientBoxCollider.h"

FrustumCollider::FrustumCollider() : 
	BaseCollider(ColliderType::Frustum)
{
}

FrustumCollider::~FrustumCollider()
{
}

bool FrustumCollider::Collision(BaseCollider* pDst)
{
	switch (pDst->GetColliderType())
	{
	case ColliderType::Sphere:
		return CollisionSphereToFrustum(((SphereCollider*)pDst)->GetBoundingSphere(), GetBoundingFrustum());
	case ColliderType::Box:
		return CollisionBoxToFrustum(((BoxCollider*)pDst)->GetBoundingBox(), GetBoundingFrustum());
	case ColliderType::OBB:
		return CollisionFrustumToOrientedBox(GetBoundingFrustum(), ((OrientBoxCollider*)pDst)->GetBoundingOBB());
	case ColliderType::Frustum:
		return CollisionFrustumToFrustum(GetBoundingFrustum(), ((FrustumCollider*)pDst)->GetBoundingFrustum());
	}

	return false;
}

void FrustumCollider::FinalUpdate()
{

}
