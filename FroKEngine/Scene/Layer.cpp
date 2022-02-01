#include "Layer.h"
#include "../Object/Object.h"

void Layer::Input(float fDeltaTime)
{
	// ������Ʈ ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{
		// ��Ȱ��ȭ��
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Input(fDeltaTime);

		// ���� �� ���̾ �׾��ٸ�
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
	// ������Ʈ ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{// ��Ȱ��ȭ��
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Update(fDeltaTime);

		// ���� �� ���̾ �׾��ٸ�
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
	// ������Ʈ ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{
		// ��Ȱ��ȭ��
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->LateUpdate(fDeltaTime);

		// ���� �� ���̾ �׾��ٸ�
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
	// ������Ʈ ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{
		// ��Ȱ��ȭ��
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Collision(fDeltaTime);

		// ���� �� ���̾ �׾��ٸ�
		if (!(*iter)->GetLife())
		{
			Object::EraseObj(*iter);
			SAFE_RELEASE((*iter));
			iter = m_ObjList.erase(iter);
			iterEnd = m_ObjList.end();
		}
		else
		{
			// �������� �ֵ鸸 �浹ü�ν� �־��ش�.
			// GET_SINGLE(CCollisionManager)->AddObject(*iter);
			++iter;
		}
	}
}

void Layer::Render(float fDeltaTime)
{
	// ������Ʈ ����Ʈ�� �ݺ��ؼ� ������ ó���� �����ϴ�.
	list<Object*>::iterator iter;
	list<Object*>::iterator iterEnd = m_ObjList.end();

	for (iter = m_ObjList.begin(); iter != iterEnd;)
	{
		// ��Ȱ��ȭ��
		if (!(*iter)->GetEnable())
		{
			++iter;
			continue;
		}

		(*iter)->Render(fDeltaTime);

		// ���� �� ���̾ �׾��ٸ�
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
	pObj->SetScene(m_pScene);	// ���̾ �Ҽӵ� ���
	pObj->SetLayer(this);		// �� ���̾ ����
	pObj->AddRef();				// ���۷��� ī���� ����

	// ������Ʈ ����Ʈ�� �߰��Ѵ�.
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
