#include "pch.h"
#include "TestObjectScript.h"
#include "Transform.h"
#include "GameObject.h"
#include "Input.h"
#include "Timer.h"

#include "Resources.h"
#include "MeshRenderer.h"

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

	if (INPUT->GetButtonDown(KEY_TYPE::KEY_1))
	{
		auto meshRenderer = GetGameObject()->GetMeshRenderer();
		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject WireFrame");
		meshRenderer->SetMaterial(material->Clone());
	}

	if (INPUT->GetButtonUp(KEY_TYPE::KEY_1))
	{
		auto meshRenderer = GetGameObject()->GetMeshRenderer();
		shared_ptr<Material> material = GET_SINGLE(Resources)->Get<Material>(L"GameObject");
		meshRenderer->SetMaterial(material->Clone());
	}

	GetTransform()->SetLocalPosition(pos);
}
