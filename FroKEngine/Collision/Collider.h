#pragma once

#include "../Ref.h"

class Collider : 
	public Ref
{
public:
	/* 게임 요소를 위한 메소드 */
	virtual bool Init() = 0;
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual bool Collision(Collider* pDst);
	virtual void Render(HDC hDC, float fDeltaTime);
	virtual Collider* Clone() = 0;
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile);

protected:
	bool CollisionRectToRect(const AABB& tBox1, const AABB& tBox2);

protected:
	Collider();
	Collider(const Collider& col);
	virtual ~Collider() = 0;

public:
	/* Setter */
	void SetObj(class GameObject* obj)
	{
		m_pObj = obj;
	}

	void SetHitPoint(const XMVECTOR& tPos)
	{
		m_tHitPoint = tPos;
	}

	/* Getter */
	COLLIDER_TYPE GetColliderType() const
	{
		return m_eColType;
	}

	class GameObject* GetObj() const
	{
		return m_pObj;
	}

	XMVECTOR GetHitPoint() const
	{
		return m_tHitPoint;
	}

public:
	void AddCollider(Collider* pCollider)
	{
		m_CollisionList.push_back(pCollider);
	}

	bool CheckCollisionList(Collider* pCollider)
	{
		list<Collider*>::iterator iter;
		list<Collider*>::iterator iterEnd = m_CollisionList.end();

		for (iter = m_CollisionList.begin(); iter != iterEnd; ++iter)
		{
			if (*iter == pCollider)
			{
				return true;
			}
		}

		return false;
	}

	void EraseCollsionList(Collider* pCollider)
	{
		list<Collider*>::iterator iter;
		list<Collider*>::iterator iterEnd = m_CollisionList.end();

		for (iter = m_CollisionList.begin(); iter != iterEnd; ++iter)
		{
			if (*iter == pCollider)
			{
				m_CollisionList.erase(iter);
				break;
			}
		}
	}

	void AddCollisionFunction(COLLISION_STATE eState,
		void(*pFunc)(Collider*, Collider*, float))
	{
		function<void(Collider*, Collider*, float)> func;

		func = bind(pFunc, placeholders::_1, placeholders::_2, placeholders::_3);

		m_FuncList[eState].push_back(func);
	}

	template <typename T>
	void AddCollisionFunction(COLLISION_STATE eState, T* pObj,
		void(T::* pFunc)(Collider*, Collider*, float))
	{
		function<void(Collider*, Collider*, float)> func;

		// pObj : 호출 객체
		func = bind(pFunc, pObj, placeholders::_1, placeholders::_2, placeholders::_3);

		m_FuncList[eState].push_back(func);
	}


	void CallFunction(COLLISION_STATE eState, Collider* pDest, float fDeltaTime)
	{
		list<function<void(Collider*, Collider*, float)>>::iterator iter;
		list<function<void(Collider*, Collider*, float)>>::iterator iterEnd = m_FuncList[eState].end();

		for (iter = m_FuncList[eState].begin(); iter != iterEnd; ++iter)
		{
			(*iter)(this, pDest, fDeltaTime);
		}
	}

protected:
	COLLIDER_TYPE	m_eColType;
	class GameObject* m_pObj;
	list<Collider*>		m_CollisionList;
	// C++11
	// 다양한 충돌체에 관련된 함수 저장소(callback)
	list<function<void(Collider*, Collider*, float)>> m_FuncList[CS_END];
	XMVECTOR m_tHitPoint;

private:
	friend class CObj;
};

