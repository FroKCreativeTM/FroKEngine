#include "pch.h"
#include "HeightFieldCollider.h"

HeightFieldCollider::HeightFieldCollider() : 
	BaseCollider(ColliderType::HeightField)
{
	geometry = geom.heightField();
}

HeightFieldCollider::~HeightFieldCollider()
{
}

void HeightFieldCollider::FinalUpdate()
{
}
