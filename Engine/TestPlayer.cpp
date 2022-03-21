#include "pch.h"
#include "TestPlayer.h"
#include "Transform.h"
#include "Camera.h"
#include "GameObject.h"
#include "CollisionManager.h"
#include "Input.h"

void TestPlayer::Update()
{
	if (INPUT->GetButtonDown(KEY_TYPE::LBUTTON))
	{
		const POINT& pos = INPUT->GetMousePos();

		auto p = GET_SINGLE(CollisionManager)->PickUI(pos.x, pos.y);

		if (p == nullptr) return;

		if (p->GetName() == L"HP")
		{
			_HP += 100;
			cout << "_HP : " << _HP << endl;
		}
	}
}
