#pragma once
#include "Component.h"

enum class ColliderType
{
	Box,
	Sphere,
	Frustum,
	OBB,
	Triangle,
	Ray,
	Plane,
	Capsule,
	HeightField,
};

class BaseCollider : public Component
{
public:
	BaseCollider(ColliderType colliderType);
	virtual ~BaseCollider();

	// virtual void FinalUpdate() override;
	virtual bool Collision(BaseCollider* pDst);

public:
	// 크게 Box, Sphere, Frustum, OrientBox, Triangle, Ray, Plane, Capsule(일단 구현할 수 있는 거부터 차례대로 구현)
	
	// *************************************************************
	//						B	O	X
	// *************************************************************
	bool CollisionBoxToBox(const BoundingBox& src, const BoundingBox& dst);
	bool CollisionBoxToSphere(const BoundingBox& src, const BoundingSphere& dst);
	bool CollisionBoxToFrustum(const BoundingBox& src, const BoundingFrustum& dst);
	bool CollisionBoxToOrientedBox(const BoundingBox& src, const BoundingOrientedBox& dst);
	bool CollisionBoxToTriangle(const BoundingBox& src, const Vec4& v0, const Vec4& v1, const Vec4& v2);
	bool CollisionBoxToRay(const BoundingBox& src, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance);
	bool CollisionBoxToPlane(const BoundingBox& src, const Vec4& dst);

	// *************************************************************
	//						S P H E R E
	// *************************************************************
	bool CollisionSphereToSphere(const BoundingSphere& src, const BoundingSphere& dst);
	bool CollisionSphereToFrustum(const BoundingSphere& src, const BoundingFrustum& dst);
	bool CollisionSphereToOrientedBox(const BoundingSphere& src, const BoundingOrientedBox& dst);
	bool CollisionSphereToTriangle(const BoundingSphere& src, const Vec4& v0, const Vec4& v1, const Vec4& v2);
	bool CollisionSphereToRay(const BoundingSphere& src, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance);
	bool CollisionSphereToPlane(const BoundingSphere& src, const Vec4& dst);

	// *************************************************************
	//						F R U S T U M
	// *************************************************************
	bool CollisionFrustumToFrustum(const BoundingFrustum& src, const BoundingFrustum& dst);
	bool CollisionFrustumToOrientedBox(const BoundingFrustum& src, const BoundingOrientedBox& dst);
	bool CollisionFrustumToTriangle(const BoundingFrustum& src, const Vec4& v0, const Vec4& v1, const Vec4& v2);
	bool CollisionFrustumToRay(const BoundingFrustum& src, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance);
	bool CollisionFrustumToPlane(const BoundingFrustum& src, const Vec4& dst);

	// *************************************************************
	//					O R I E N T E D B O X
	// *************************************************************
	bool CollisionOrientedBoxToOrientedBox(const BoundingOrientedBox& src, const BoundingOrientedBox& dst);
	bool CollisionOrientedBoxToTriangle(const BoundingOrientedBox& src, const Vec4& v0, const Vec4& v1, const Vec4& v2);
	bool CollisionOrientedBoxToRay(const BoundingOrientedBox& src, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance);
	bool CollisionOrientedBoxToPlane(const BoundingOrientedBox& src, const Vec4& dst);

	// *************************************************************
	//  				  T R I A N G L E
	// *************************************************************
	bool CollisionTriangleToTriangle(const Vec4& v10, const Vec4& v11, const Vec4& v12, 
		const Vec4& v20, const Vec4& v21, const Vec4& v22);
	bool CollisionTriangleToRay(const Vec4& v0, const Vec4& v1, const Vec4& v2, Vec4 rayOrigin, Vec4 rayDir, OUT float& distance);
	bool CollisionTriangleToPlane(const Vec4& v0, const Vec4& v1, const Vec4& v2, const Vec4& dst);

	// *************************************************************
	//							R A Y
	// *************************************************************
	bool CollisionRayToPlane(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance, const Vec4& dst);

	// *************************************************************
	//						  P L A N E
	// *************************************************************
	bool CollisionPlaneToPlane(const Vec4& src, const Vec4& dst);

public:
	bool GetTriggerd() const { return _isTriggerd; }
	void SetTriggered(bool b) { _isTriggerd = b; }

	physx::PxShape* GetShape() { return _colliderShape; }
	ColliderType GetColliderType() { return _colliderType; }

protected :
	bool _isTriggerd = true;

protected:
	ColliderType		_colliderType = {};
	physx::PxShape*		_colliderShape = nullptr;
};