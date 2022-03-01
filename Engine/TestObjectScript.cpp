#include "pch.h"
#include "TestObjectScript.h"
#include "Transform.h"
#include "GameObject.h"
#include "Input.h"
#include "Timer.h"

TestObjectScript::TestObjectScript()
{
}

TestObjectScript::~TestObjectScript()
{
}

void TestObjectScript::LateUpdate()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	if (INPUT->GetButton(KEY_TYPE::W))
		pos += GetTransform()->GetWorldPosition() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::S))
		pos -= GetTransform()->GetWorldPosition() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::A))
		pos -= GetTransform()->GetWorldPosition() * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::D))
		pos += GetTransform()->GetWorldPosition() * _speed * DELTA_TIME;

	GetTransform()->SetLocalPosition(pos);
}
