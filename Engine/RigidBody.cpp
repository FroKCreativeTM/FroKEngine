#include "pch.h"
#include "Rigidbody.h"
#include "GameObject.h"
#include "Transform.h"
#include "Timer.h"
#include "Physics.h"

#include "BaseCollider.h"

RigidBody::RigidBody() : Component(COMPONENT_TYPE::RIGIDBODY)
{
}

RigidBody::~RigidBody()
{
}

void RigidBody::Init(BaseCollider* col)
{
}

void RigidBody::FinalUpdate()
{
}
