#include "Layer.h"
#include "../Object/Object.h"

void Layer::Input(float fDeltaTime)
{
	// 오브젝트 리스트를 반복해서 돌려야 처리가 가능하다.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{
		// 비활성화시
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Input(fDeltaTime);

		// 만약 이 레이어가 죽었다면
		if (!(*iter)->GetLife())
		{
			Object::EraseObj(*iter);
			SAFE_RELEASE((*iter));
			iter = m_ObjList.erase(iter);
			iterEnd = m_ObjList.end();
		}
		else
		{
			++iter;
		}
	}
}

int Layer::Update(float fDeltaTime)
{
	// 오브젝트 리스트를 반복해서 돌려야 처리가 가능하다.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{// 비활성화시
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Update(fDeltaTime);

		// 만약 이 레이어가 죽었다면
		if (!(*iter)->GetLife())
		{
			Object::EraseObj(*iter);
			SAFE_RELEASE((*iter));
			iter = m_ObjList.erase(iter);
			iterEnd = m_ObjList.end();
		}
		else
		{
			++iter;
		}
	}

	return 0;
}

int Layer::LateUpdate(float fDeltaTime)
{
	// 오브젝트 리스트를 반복해서 돌려야 처리가 가능하다.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{
		// 비활성화시
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->LateUpdate(fDeltaTime);

		// 만약 이 레이어가 죽었다면
		if (!(*iter)->GetLife())
		{
			Object::EraseObj(*iter);
			SAFE_RELEASE((*iter));
			iter = m_ObjList.erase(iter);
			iterEnd = m_ObjList.end();
		}
		else
		{
			++iter;
		}
	}


	return 0;
}

void Layer::Collision(float fDeltaTime)
{
	// 오브젝트 리스트를 반복해서 돌려야 처리가 가능하다.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{
		// 비활성화시
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Collision(fDeltaTime);

		// 만약 이 레이어가 죽었다면
		if (!(*iter)->GetLife())
		{
			Object::EraseObj(*iter);
			SAFE_RELEASE((*iter));
			iter = m_ObjList.erase(iter);
			iterEnd = m_ObjList.end();
		}
		else
		{
			// 인지워진 애들만 충돌체로써 넣어준다.
			// GET_SINGLE(CCollisionManager)->AddObject(*iter);
			++iter;
		}
	}
}

void Layer::Render(float fDeltaTime)
{
	// 오브젝트 리스트를 반복해서 돌려야 처리가 가능하다.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{
		// 비활성화시
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Render(fDeltaTime);

		// 만약 이 레이어가 죽었다면
		if (!(*iter)->GetLife())
		{
			Object::EraseObj(*iter);
			SAFE_RELEASE((*iter));
			iter = m_ObjList.erase(iter);
			iterEnd = m_ObjList.end();
		}
		else
		{
			++iter;
		}
	}
}

void Layer::AddObj(Object* pObj)
{
	pObj->SetScene(m_pScene);	// 레이어가 소속된 장면
	pObj->SetLayer(this);		// 이 레이어를 전달
	pObj->AddRef();				// 레퍼런스 카운터 증가

	// 오브젝트 리스트에 추가한다.
	m_ObjList.push_back(pObj);
}

Layer::Layer() :
	m_strTag(""),
	m_pScene(nullptr),
	m_bEnable(true),
	m_bLife(true)
{
}

Layer::~Layer()
{
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd; ++iter)
	{
		Object::EraseObj(*iter);
		SAFE_RELEASE((*iter));
	}

	m_ObjList.clear();
}
