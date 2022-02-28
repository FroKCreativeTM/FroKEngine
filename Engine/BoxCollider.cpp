#include "pch.h"
#include "BoxCollider.h"
#include "GameObject.h"
#include "Transform.h"

BoxCollider::BoxCollider() : BaseCollider(ColliderType::Box)
{
}

BoxCollider::~BoxCollider()
{
}

void BoxCollider::FinalUpdate()
{
	_boundingBox.Center = GetGameObject()->GetTransform()->GetWorldPosition();

	Vec3 scale = GetGameObject()->GetTransform()->GetLocalScale();

	_boundingBox.Extents.x = scale.x;
	_boundingBox.Extents.y = scale.y;
	_boundingBox.Extents.z = scale.z;
}

bool BoxCollider::Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return _boundingBox.Intersects(rayOrigin, rayDir, OUT distance);
}
