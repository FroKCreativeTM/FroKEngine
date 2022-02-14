#include "Object.h"
#include "../Scene/SceneManager.h"
#include "../Scene/Scene.h"
#include "../Resource/ResourceManager.h"
#include "../Graphics/FrameResource.h"
#include "../Path/PathManager.h"

#include "../Collision/RectCollider.h"

// static
list<Object*> Object::m_ObjList;

Object::Object()
{
}

Object::Object(const Object& ref)
{
	*this = ref;
	m_nRef = 1;
}

Object::~Object()
{
}

void Object::Input(float fDeltaTime)
{
}

int Object::Update(float fDeltaTime)
{
	/* 충돌체를 돌린다. */
	list<Collider*>::iterator iter;
	list<Collider*>::iterator iterEnd = m_ColliderList.end();

	for (iter = m_ColliderList.begin(); iter != iterEnd;)
	{
		// 콜라이더가 비활성화 되어있다면
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}
		(*iter)->Update(fDeltaTime);

		if (!(*iter)->GetLife())
		{
			SAFE_RELEASE((*iter));
			iter = m_ColliderList.erase(iter);
			iterEnd = m_ColliderList.end();
		}
		else
			++iter;
	}

    return 0;
}

int Object::LateUpdate(float fDeltaTime)
{
	list<Collider*>::iterator iter;
	list<Collider*>::iterator iterEnd = m_ColliderList.end();

	for (iter = m_ColliderList.begin(); iter != iterEnd;)
	{
		// 콜라이더가 비활성화 되어있다면
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}
		(*iter)->LateUpdate(fDeltaTime);

		if (!(*iter)->GetLife())
		{
			SAFE_RELEASE((*iter));
			iter = m_ColliderList.erase(iter);
			iterEnd = m_ColliderList.end();
		}
		else
			++iter;
	}

    return 0;
}

void Object::Collision(float fDeltaTime)
{
}

void Object::Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime)
{

}

Collider* Object::GetCollider(const string& strTag)
{
	list<Collider*>::iterator iter;
	list<Collider*>::iterator iterEnd = m_ColliderList.end();

	for (iter = m_ColliderList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter)->GetTag() == strTag)
		{
			(*iter)->AddRef();
			return *iter;
		}
	}

	return nullptr;
}

void Object::Save(FILE* pFile)
{
}

void Object::Load(FILE* pFile)
{
}

void Object::SaveFromPath(const char* pFileName, const string& strPathKey)
{
}

void Object::SaveFromFullPath(const char* pFullPath)
{
}

void Object::LoadFromPath(const char* pFileName, const string& strPathKey)
{
}

void Object::LoadFromFullPath(const char* pFullPath)
{
}

Object* Object::CreateCloneObj(const string& strProtoKey, const string& strTag, 
	SCENE_CREATE scr)
{
	// 어떤 타입인지 알아서 찾아서 복사하도록
	// 짜피 static이라 CScene::
	Object* pPrototype = Scene::FindPrototype(strProtoKey);

	if (!pPrototype)
	{
		return nullptr;
	}

	Object* pObj = pPrototype->Clone();
	pObj->SetTag(strTag);

	AddObj(pObj);

	return pObj;
}

void Object::AddObj(Object* pObj)
{
    // 인자로 들어온 물건들을 다 넣어준다.
    pObj->AddRef();
    m_ObjList.push_back(pObj);
}

Object* Object::FindObj(const string& strTag)
{
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter)->GetTag() == strTag)
		{
			(*iter)->AddRef();
			return *iter;
		}
	}
	return nullptr;
}

void Object::EraseObj()
{
	Safe_Release_VecList(m_ObjList);
}

void Object::EraseObj(Object* pObj)
{
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	// 처음부터 끝까지 돌면서 찾으면 그것을 지운다.
	for (iter = m_ObjList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter) == pObj)
		{
			SAFE_RELEASE((*iter));
			iter = m_ObjList.erase(iter);
			return;
		}
	}
}

void Object::EraseObj(const string& strTag)
{
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter)->GetTag() == strTag)
		{
			SAFE_RELEASE((*iter));
			iter = m_ObjList.erase(iter);
			return;
		}
	}
}
