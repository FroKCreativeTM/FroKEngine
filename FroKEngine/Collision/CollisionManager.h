#pragma once

#include "../Game.h"

class CollisionManager
{
	DECLARE_SINGLE(CollisionManager)

public:
	void AddObject(class GameObject* pObj);
	void Collision(float fDeltaTime);

	// 두 오브젝트의 충돌 처리를 담당한다.
	bool Collision(GameObject* pSrc, GameObject* pDst, float fDeltaTime);

	// 씬 변화 등등의 경우를 위해 충돌체를 모두 지워준다.
	void ClearScene();

private:
	// 충돌체가 있는 오브젝트들로 리스트를 채우는 방식
	list<class GameObject*> m_CollisionList;
};

