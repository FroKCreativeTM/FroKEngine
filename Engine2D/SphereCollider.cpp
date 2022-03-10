#include "pch.h"
#include "SphereCollider.h"
#include "BoxCollider.h"

#include "GameObject.h"
#include "Transform.h"


SphereCollider::SphereCollider() : BaseCollider(ColliderType::Sphere)
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
	}

	return false;
}

void SphereCollider::FinalUpdate()
{

	_boundingSphere.Center = GetGameObject()->GetTransform()->GetWorldPosition();

	Vec3 scale = GetGameObject()->GetTransform()->GetLocalScale();
	_sphere.radius = _radius * max(max(scale.x, scale.y), scale.z);
	_boundingSphere.Radius = _radius * max(max(scale.x, scale.y), scale.z);
}
//
//bool SphereCollider::Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
//{
//	// �ݸ��� �ɼ��� ���� �ִ� ��쿡�� ����ǰ�
//	if (_isTriggerd)	
//		return _boundingSphere.Intersects(rayOrigin, rayDir, OUT distance);
//	else
//		return false;
//}