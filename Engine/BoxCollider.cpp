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

	_size = scale;
	_boundingBox.Extents = scale;
}

bool BoxCollider::Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	if (_isTriggerd)
		return _boundingBox.Intersects(rayOrigin, rayDir, OUT distance);
	else
		return false;
}
