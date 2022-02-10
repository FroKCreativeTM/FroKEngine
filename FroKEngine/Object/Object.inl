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

template<typename T>
inline void Object::AddCollisionFunction(const string& strTag,
	COLLISION_STATE eState, T* pObj, void(T::* pFunc)(Collider*, Collider*, float))
{
	list<CCollider*>::iterator iter;
	list<CCollider*>::iterator iterEnd = m_ColliderList.end();

	for (iter = m_ColliderList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter)->GetTag() == strTag)
		{
			(*iter)->AddCollisionFunction(eState, pObj, pFunc);
			break;
		}
	}
}