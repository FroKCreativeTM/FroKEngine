#include "pch.h"
#include "TestUIMono.h"
#include "Transform.h"
#include "GameObject.h"
#include "Input.h"

TestUIMono::TestUIMono()
{
}

TestUIMono::~TestUIMono()
{
}

void TestUIMono::LateUpdate()
{
	if (INPUT->GetButton(KEY_TYPE::ESC))
	{
		bool b = GetGameObject()->GetLife();
		GetGameObject()->SetLife(!b);
	}
}
