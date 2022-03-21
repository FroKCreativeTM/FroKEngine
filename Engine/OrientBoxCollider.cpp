#include "pch.h"
#include "OrientBoxCollider.h"

#include "GameObject.h"
#include "Transform.h"

#include "BoxCollider.h"
#include "FrustumCollider.h"
#include "SphereCollider.h"

OrientBoxCollider::OrientBoxCollider() : 
    BaseCollider(ColliderType::OBB)
{
}

OrientBoxCollider::~OrientBoxCollider()
{
}

bool OrientBoxCollider::Collision(BaseCollider* pDst)
{
	switch (pDst->GetColliderType())
	{
	case ColliderType::Sphere:
		return CollisionSphereToOrientedBox(((SphereCollider*)pDst)->GetBoundingSphere(), GetBoundingOBB());
	case ColliderType::Box:
		return CollisionBoxToOrientedBox(((BoxCollider*)pDst)->GetBoundingBox(), GetBoundingOBB());
	case ColliderType::OBB:
		return CollisionOrientedBoxToOrientedBox(GetBoundingOBB(), ((OrientBoxCollider*)pDst)->GetBoundingOBB());
	case ColliderType::Frustum:
		return CollisionFrustumToOrientedBox(((FrustumCollider*)pDst)->GetBoundingFrustum(), GetBoundingOBB());
	}

	return false;
}

void OrientBoxCollider::FinalUpdate()
{
	_boundingOOB.Center = GetGameObject()->GetTransform()->GetWorldPosition();

	Vec3 scale = GetGameObject()->GetTransform()->GetLocalScale();
	_boundingOOB.Extents = scale;
}

void OrientBoxCollider::Render()
{
	// DX::Draw()
}
