#pragma once

#include "../Game.h"

class CollisionManager
{
	DECLARE_SINGLE(CollisionManager)

public:
	void AddObject(class Object* pObj);
	void Collision(float fDeltaTime);

	// �� ������Ʈ�� �浹 ó���� ����Ѵ�.
	bool Collision(Object* pSrc, Object* pDst, float fDeltaTime);

	// �� ��ȭ ����� ��츦 ���� �浹ü�� ��� �����ش�.
	void ClearScene();

private:
	// �浹ü�� �ִ� ������Ʈ��� ����Ʈ�� ä��� ���
	std::list<class Object*> m_CollisionList;
};

