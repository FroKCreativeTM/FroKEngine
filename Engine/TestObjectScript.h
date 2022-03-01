#pragma once
#include "MonoBehaviour.h"

class TestObjectScript :
    public MonoBehaviour
{
public:
	TestObjectScript();
	virtual ~TestObjectScript();

	virtual void LateUpdate() override;

private:
	float		_speed = 100.f;
};

