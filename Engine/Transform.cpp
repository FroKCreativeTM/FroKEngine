#include "pch.h"
#include "Transform.h"
#include "Engine.h"
#include "Camera.h"

Transform::Transform() : Component(COMPONENT_TYPE::TRANSFORM)
{

}

Transform::~Transform()
{

}

/// <summary>
/// 물체의 상태와 위치값을 마지막에 갱신한다.
/// </summary>
void Transform::FinalUpdate()
{
	Matrix matScale = Matrix::CreateScale(_localScale);
	// 짐벌락
	Matrix matRotation = Matrix::CreateRotationX(_localRotation.x);
	matRotation *= Matrix::CreateRotationY(_localRotation.y);
	matRotation *= Matrix::CreateRotationZ(_localRotation.z);
	Matrix matTranslation = Matrix::CreateTranslation(_localPosition);

	_matLocal = matScale * matRotation * matTranslation;
	_matWorld = _matLocal;

	shared_ptr<Transform> parent = GetParent().lock();
	if (parent != nullptr)
	{
		_matWorld *= parent->GetLocalToWorldMatrix();
	}
}

void Transform::PushData()
{
	// World		: 
	// View			: 카메라가 바라보는 방향과 좌표를 기준
	// Projection	: 

	Matrix matWVP = _matWorld * Camera::S_MatView * Camera::S_MatProjection;
	CONST_BUFFER(CONSTANT_BUFFER_TYPE::TRANSFORM)->PushData(&_matWorld, sizeof(_matWorld));
}