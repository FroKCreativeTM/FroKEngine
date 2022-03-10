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

	if (INPUT->GetButton(KEY_TYPE::I))
		pos.x += GetTransform()->GetWorldPosition().x * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::K))
		pos.x -= GetTransform()->GetWorldPosition().x * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::J))
		pos.y -= GetTransform()->GetWorldPosition().y * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::L))
		pos.y += GetTransform()->GetWorldPosition().y * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::J))
		pos.z -= GetTransform()->GetWorldPosition().z * _speed * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::L))
		pos.z += GetTransform()->GetWorldPosition().z * _speed * DELTA_TIME;

	GetTransform()->SetLocalPosition(pos);
}
