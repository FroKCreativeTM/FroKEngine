#pragma once

#include "../Game.h"

class CollisionManager
{
	DECLARE_SINGLE(CollisionManager)

public:
	void AddObject(class GameObject* pObj);
	void Collision(float fDeltaTime);

	// �� ������Ʈ�� �浹 ó���� ����Ѵ�.
	bool Collision(GameObject* pSrc, GameObject* pDst, float fDeltaTime);

	// �� ��ȭ ����� ��츦 ���� �浹ü�� ��� �����ش�.
	void ClearScene();

private:
	// �浹ü�� �ִ� ������Ʈ��� ����Ʈ�� ä��� ���
	list<class GameObject*> m_CollisionList;
};

