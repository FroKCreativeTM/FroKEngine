#pragma once

#include "Object.h"
#include "../Collision/Collider.h"

class GameObject : 
	public Object
{
private : 
// ��ü�� �ڷᰡ ���ؼ� ��� ���۸� �����ؾ� �ϴ� ���� ���θ� ���ϴ� ��Ƽ �÷����̴�.
// FrameResource���� ��ü�� cbuffer�� �����Ƿ�, FrameResource���� ������ �����ؾ� �Ѵ�.
// ���� ��ü�� �ڷḦ ������ ������ �ݵ��
// NumFrameDirty = gNumFrameResources�� �����ؾ��Ѵ�.
// �׷��� ������ ������ �ڿ��� ���ŵȴ�.
	int nFramesDirty = gNumFrameResource;

	// �� ���� �������� ��ü ��� ���ۿ� �ش��ϴ� GPU ��� ������ ����
	UINT objCBIdx = -1;

	// �� ���� �׸� ������ ���� ������ ���׸����� �����Ѵ�. 
	// ���� ������ �׸��� ���� ���� ������ ������ �� ������ �����϶�.
	class Material* Mat = nullptr;
	MeshGeometry* pGeometry = nullptr;

	// �⺻ ���� ���� ����
	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced �Ű�������
	UINT nIdxCnt = 0;
	UINT nStartIdxLocation = 0;
	int nBaseVertexLocation = 0;

	// �浹ü�� ���� ����
	list<Collider*>		m_ColliderList;

protected:
	GameObject();
	GameObject(const GameObject& ref);
	virtual ~GameObject();

public : 
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual void Collision(float fDeltaTime);
	virtual void Render(float fDeltaTime);
	virtual Object* Clone() = 0;

public : 
	/* Getter */
	Collider* GetCollider(const string& strTag);

	bool CheckCollider()
	{
		// �浹�� �ֳ������� �˻��Ѵ�.
		return !m_ColliderList.empty();
	}

	// ����Ʈ �����͸� �ѱ��.
	const list<Collider*>* GetColliderList() const
	{
		return &m_ColliderList;
	}

	template <typename T>
	void AddCollisionFunction(const string& strTag,
		COLLISION_STATE eState, T* pObj,
		void(T::* pFunc)(Collider*, Collider*, float));

	template <typename T>
	T* AddCollider(const string& strTag)
	{
		// �浹ü�� �����ϰ� �� �浹ü�� ���� ������Ʈ�� ������ �����Ѵ�
		T* pCollider = new T;
		pCollider->SetObj(this);
		pCollider->SetTag(strTag);

		if (!pCollider->Init())
		{
			SAFE_RELEASE(pCollider);
			return nullptr;
		}

		pCollider->AddRef();
		m_ColliderList.push_back(pCollider);

		return pCollider;
	}
};