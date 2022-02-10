#pragma once

#include "../Scene/Layer.h"
#include "Object.h"

template <typename T>
static T* Object::CreateObj(const string& strTag,
	class Layer* pLayer)
{
	T* pObj = new T;

	pObj->SetTag(strTag);

	if (!pObj->Init())
	{
		SAFE_RELEASE(pObj);
		return nullptr;
	}

	if (pLayer)
	{
		pLayer->AddObj(pObj);
	}

	AddObj(pObj);

	return pObj;
}
