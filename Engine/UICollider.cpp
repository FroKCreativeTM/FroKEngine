#include "pch.h"
#include "UICollider.h"
#include "Transform.h"
#include "Camera.h"

UICollider::UICollider() : 
	BaseCollider(ColliderType::UI)
{
}

UICollider::~UICollider()
{
}

bool UICollider::Collision(int32 x, int32 y)
{
	//if (_info.left < x)
	//{
	//	return false;
	//}
	//else if (_info.right > x)
	//{
	//	return false;
	//}
	//else if (_info.top < y)
	//{
	//	return false;
	//}
	//else if (_info.bottom > y)
	//{
	//	return false;
	//}
	//return true;
	return false;
}

void UICollider::FinalUpdate()
{
	_center = GetTransform()->GetLocalPosition();
}
