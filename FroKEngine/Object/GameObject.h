#pragma once

#include "Object.h"
#include "../Collision/Collider.h"

class GameObject : 
	public Object
{
private : 
// 물체의 자료가 변해서 상수 버퍼를 갱신해야 하는 지의 여부를 뜻하는 더티 플래그이다.
// FrameResource마다 물체의 cbuffer가 있으므로, FrameResource마다 갱신을 적용해야 한다.
// 따라서 물체의 자료를 수정할 때에는 반드시
// NumFrameDirty = gNumFrameResources로 설정해야한다.
// 그래야 각각의 프레임 자원이 갱신된다.
	int nFramesDirty = gNumFrameResource;

	// 이 렌더 아이템의 물체 상수 버퍼에 해당하는 GPU 상수 버퍼의 색인
	UINT objCBIdx = -1;

	// 이 렌더 항목에 연관된 기하 구조및 마테리얼을 연결한다. 
	// 여러 렌더가 항목이 같은 기하 구조를 참조할 수 있음을 주의하라.
	class Material* Mat = nullptr;
	MeshGeometry* pGeometry = nullptr;

	// 기본 도형 위상 구조
	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced 매개변수들
	UINT nIdxCnt = 0;
	UINT nStartIdxLocation = 0;
	int nBaseVertexLocation = 0;

	// 충돌체에 대한 정보
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
		// 충돌이 있나없나를 검사한다.
		return !m_ColliderList.empty();
	}

	// 리스트 포인터를 넘긴다.
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
		// 충돌체를 생성하고 이 충돌체를 가질 오브젝트의 정보를 전달한다
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