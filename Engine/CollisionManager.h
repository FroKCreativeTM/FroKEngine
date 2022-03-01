#pragma once

class BaseCollider;
class BoxCollider;
class SphereCollider;
class CapsuleCollider;

class GameObject;

class CollisionManager
{
	DECLARE_SINGLE(CollisionManager)

public : 
	// 여기서 전부 검출하면...될려나?
	void AddObject(shared_ptr<GameObject> pObj);
	void FinalUpdate();

	// 두 오브젝트의 충돌 처리를 담당한다.
	bool Collision(GameObject pSrc, GameObject pDst);

public :
	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);

public :

private :
	// 충돌체가 있는 오브젝트들로 리스트를 채우는 방식
	std::list<shared_ptr<GameObject>> m_CollisionList;
};
