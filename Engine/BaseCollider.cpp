#include "pch.h"
#include "BaseCollider.h"

BaseCollider::BaseCollider(ColliderType colliderType)
	: Component(COMPONENT_TYPE::COLLIDER), _colliderType(colliderType)
{

}

BaseCollider::~BaseCollider()
{

}

bool BaseCollider::CollisionBoxToBox(const BoundingBox& src, const BoundingBox& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionBoxToSphere(const BoundingBox& src, const BoundingSphere& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionBoxToFrustum(const BoundingBox& src, const BoundingFrustum& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionBoxToOrientedBox(const BoundingBox& src, const BoundingOrientedBox& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionBoxToTriangle(const BoundingBox& src, const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
	return src.Intersects(v0, v1, v2);
}

bool BaseCollider::CollisionBoxToRay(const BoundingBox& src, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return src.Intersects(rayOrigin, rayDir, distance);
}

bool BaseCollider::CollisionBoxToPlane(const BoundingBox& src, const Vec4& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionSphereToSphere(const BoundingSphere& src, const BoundingSphere& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionSphereToFrustum(const BoundingSphere& src, const BoundingFrustum& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionSphereToOrientedBox(const BoundingSphere& src, const BoundingOrientedBox& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionSphereToTriangle(const BoundingSphere& src, const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
	return src.Intersects(v0, v1, v2);
}

bool BaseCollider::CollisionSphereToRay(const BoundingSphere& src, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return src.Intersects(rayOrigin, rayDir, distance);
}

bool BaseCollider::CollisionSphereToPlane(const BoundingSphere& src, const Vec4& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionFrustumToFrustum(const BoundingFrustum& src, const BoundingFrustum& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionFrustumToOrientedBox(const BoundingFrustum& src, const BoundingOrientedBox& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionFrustumToTriangle(const BoundingFrustum& src, const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
	return src.Intersects(v0, v1, v2);
}

bool BaseCollider::CollisionFrustumToRay(const BoundingFrustum& src, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return src.Intersects(rayOrigin, rayDir, distance);
}

bool BaseCollider::CollisionFrustumToPlane(const BoundingFrustum& src, const Vec4& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionOrientedBoxToOrientedBox(const BoundingOrientedBox& src, const BoundingOrientedBox& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionOrientedBoxToTriangle(const BoundingOrientedBox& src, const Vec4& v0, const Vec4& v1, const Vec4& v2)
{
	return src.Intersects(v0, v1, v2);
}

bool BaseCollider::CollisionOrientedBoxToRay(const BoundingOrientedBox& src, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return src.Intersects(rayOrigin, rayDir, distance);
}

bool BaseCollider::CollisionOrientedBoxToPlane(const BoundingOrientedBox& src, const Vec4& dst)
{
	return src.Intersects(dst);
}

bool BaseCollider::CollisionTriangleToTriangle(const Vec4& v10, const Vec4& v11, const Vec4& v12, 
	const Vec4& v20, const Vec4& v21, const Vec4& v22)
{
	return true;
}

bool BaseCollider::CollisionTriangleToRay(const Vec4& v0, const Vec4& v1, const Vec4& v2, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
	return true;
}

bool BaseCollider::CollisionTriangleToPlane(const Vec4& v0, const Vec4& v1, const Vec4& v2, const Vec4& dst)
{
	return true;
}

bool BaseCollider::CollisionRayToPlane(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance, const Vec4& dst)
{
	return true;
}

bool BaseCollider::CollisionPlaneToPlane(const Vec4& src, const Vec4& dst)
{
	return true;
}
