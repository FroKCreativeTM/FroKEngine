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
	void FinalUpdate();

public :
	shared_ptr<class GameObject> Pick(int32 screenX, int32 screenY);

private :
	// �浹ü�� �ִ� ������Ʈ��� ����Ʈ�� ä��� ���
	std::list<weak_ptr<GameObject>> m_CollisionList;
};
