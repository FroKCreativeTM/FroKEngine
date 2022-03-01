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
	void FinalUpdate();

public :
	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);

private :
	// 충돌체가 있는 오브젝트들로 리스트를 채우는 방식
	std::list<weak_ptr<GameObject>> m_CollisionList;
};
