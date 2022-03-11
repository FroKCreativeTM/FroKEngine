#pragma once

#include "MonoBehaviour.h"

class TestUIMono :
    public MonoBehaviour
{
public:
	TestUIMono();
	virtual ~TestUIMono();

	virtual void LateUpdate() override;
};

