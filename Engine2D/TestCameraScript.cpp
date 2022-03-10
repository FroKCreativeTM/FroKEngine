#include "pch.h"
#include "TestCameraScript.h"
#include "Transform.h"
#include "Camera.h"
#include "GameObject.h"
#include "Input.h"
#include "Timer.h"
#include "SceneManager.h"
#include "CollisionManager.h"

TestCameraScript::TestCameraScript()
{
}

TestCameraScript::~TestCameraScript()
{
}

void TestCameraScript::LateUpdate()
{
	Vec3 pos = GetTransform()->GetLocalPosition();

	if (INPUT->GetButton(KEY_TYPE::W))
		pos.y += GetTransform()->GetWorldPosition().y * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::S))
		pos.y -= GetTransform()->GetWorldPosition().y * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::A))
		pos.x -= GetTransform()->GetWorldPosition().x * DELTA_TIME;

	if (INPUT->GetButton(KEY_TYPE::D))
		pos.x += GetTransform()->GetWorldPosition().x * DELTA_TIME;

	GetTransform()->SetLocalPosition(pos);
}
