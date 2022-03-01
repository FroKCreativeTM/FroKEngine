#pragma once
#include "Component.h"

enum class ColliderType
{
	Sphere,
	Box,

};

class BaseCollider : public Component
{
public:
	BaseCollider(ColliderType colliderType);
	virtual ~BaseCollider();

	virtual bool Intersects(Vec4 rayOrigin, Vec4 rayDir, OUT float& distance) = 0;

	bool GetTriggerd() const { return _isTriggerd; }
	void SetTriggered(bool b) { _isTriggerd = b; }

protected :
	bool _isTriggerd = true;

private:
	ColliderType _colliderType = {};
};