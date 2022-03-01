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
	float		_speed = 1.f;
};

