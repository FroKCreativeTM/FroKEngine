#include "Object.h"

void Object::Input(float fDeltaTime)
{
}

int Object::Update(float fDeltaTime)
{
    return 0;
}

int Object::LateUpdate(float fDeltaTime)
{
    return 0;
}

void Object::Collision(float fDeltaTime)
{
}

void Object::Render(float fDeltaTime)
{
}

Object* Object::Clone()
{
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

void Object::AddObj(Object* pObj)
{
    // ���ڷ� ���� ���ǵ��� �� �־��ش�.
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

	// ó������ ������ ���鼭 ã���� �װ��� �����.
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
