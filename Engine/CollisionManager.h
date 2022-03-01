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
	// ���⼭ ���� �����ϸ�...�ɷ���?
	void AddObject(shared_ptr<GameObject> pObj);
	void FinalUpdate();

	// �� ������Ʈ�� �浹 ó���� ����Ѵ�.
	bool Collision(GameObject pSrc, GameObject pDst);

public :
	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);

public :

private :
	// �浹ü�� �ִ� ������Ʈ��� ����Ʈ�� ä��� ���
	std::list<shared_ptr<GameObject>> m_CollisionList;
};
