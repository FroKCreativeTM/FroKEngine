#include "pch.h"
#include "Rigidbody.h"
#include "GameObject.h"
#include "Transform.h"
#include "Timer.h"
#include "Physics.h"

using namespace physx;

RigidBody::RigidBody() : Component(COMPONENT_TYPE::RIGIDBODY)
{
	
}

RigidBody::~RigidBody()
{
}

void RigidBody::FinalUpdate()
{
	if (_useGravity)
	{
		Vec3 pos = GetTransform()->GetLocalPosition();
		pos.y -= GRAVITY * DELTA_TIME;
		GetTransform()->SetLocalPosition(pos);
	}
}
