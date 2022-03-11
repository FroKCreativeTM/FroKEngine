#include "pch.h"
#include "PlaneCollider.h"

#include "GameObject.h"
#include "Transform.h"

PlaneCollider::PlaneCollider() : 
    BaseCollider(ColliderType::Plane)
{
}

PlaneCollider::~PlaneCollider()
{
}

bool PlaneCollider::Collision(BaseCollider* pDst)
{
    return false;
}

bool PlaneCollider::Collision(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance)
{
    float denom = _normalVec.Dot(Vec3(rayDir.x, rayDir.y, rayDir.z));
    if (abs(denom) > 0.0001f) // your favorite epsilon
    {
        float t = (GetGameObject()->GetTransform()->GetWorldPosition() - Vec3(rayOrigin)).Dot(_normalVec) / denom;
        if (t >= 0) return true; // you might want to allow an epsilon here too
    }
    return false;
}

void PlaneCollider::FinalUpdate()
{
    _normalVec = GetGameObject()->GetTransform()->GetWorldPosition();
    _normalVec.Normalize();
}

void PlaneCollider::Render()
{
    // 
    // 
    // 
    // 
    // 
}
