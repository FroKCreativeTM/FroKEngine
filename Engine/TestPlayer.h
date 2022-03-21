#pragma once
#include "MonoBehaviour.h"
class TestPlayer :
    public MonoBehaviour
{
public:
	virtual void Update() override;

private : 
	int32 _HP = 100;
	int32 _MP = 100;
};

