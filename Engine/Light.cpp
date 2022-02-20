#include "pch.h"
#include "Light.h"
#include "Transform.h"

Light::Light() : Component(COMPONENT_TYPE::LIGHT)
{
}

Light::~Light()
{
}

void Light::FinalUpdate()
{
	_lightInfo.pos = GetTransform()->GetWorldPosition();
}
