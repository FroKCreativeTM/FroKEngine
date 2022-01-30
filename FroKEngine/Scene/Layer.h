#pragma once

#include "../Game.h"

using namespace std;

/// <summary>
/// 이 클래스는 오브젝트의 그룹을 만들고 이를 관리하기 위해 만든 클래스입니다.
/// 레이어는 씬의 일부분만을 렌더링하거나 물리적인 판정을 하기 위해서 사용되는 클래스입니다.
/// 
/// </summary>
class Layer
{
public:
	void SetTag(const string& str)
	{
		this->m_strTag = str;
	}

	void SetScene(class Scene* pScene)
	{
		this->m_pScene = pScene;
	}

	string GetTag() const
	{
		return m_strTag;
	}

	class Scene* GetScene() const
	{
		return this->m_pScene;
	}

public:
	/* 코어부터 지금 돌아가는 씬의 레이어들까지 내려왔다. */
	/* 게임 요소를 위한 메소드 */
	void Input(float fDeltaTime);
	int Update(float fDeltaTime);
	int LateUpdate(float fDeltaTime);
	void Collision(float fDeltaTime);
	void Render(float fDeltaTime);

public:
	void AddObj(class Object* pObj);

public:
	void SetEnable(bool bEnable)
	{
		this->m_bEnable = bEnable;
	}

	void SetLife(bool bLife)
	{
		this->m_bLife = bLife;
	}

	bool GetLife() const
	{
		return m_bLife;
	}

	bool GetEnable() const
	{
		return m_bEnable;
	}

private:
	friend class Scene;

	// 씬 외에는 생성하지 못하도록 하기 위해서 private로 생성
private:
	Layer();

public:
	// Safe_Delete_VecList를 위해서
	~Layer();

private:
	class Scene* m_pScene;	// 이 레이어를 가진 장면 정보를 가진다.
	string		m_strTag;
	// 오브젝트는 삭제가 많이 생기는 그런 물건이다.
	// 그렇기 때문에 중간 삽입,삭제가 굉장히 많이 발생할 것이다.
	// 그런 부분은 vector보다는 list가 빠르다.
	list<class Object*> m_ObjList;
	bool		m_bEnable;	// 레이어 활성화
	bool		m_bLife;	// 살리는가 죽이는가.
};

