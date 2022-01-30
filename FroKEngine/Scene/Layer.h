#pragma once

#include "../Game.h"

using namespace std;

/// <summary>
/// �� Ŭ������ ������Ʈ�� �׷��� ����� �̸� �����ϱ� ���� ���� Ŭ�����Դϴ�.
/// ���̾�� ���� �Ϻκи��� �������ϰų� �������� ������ �ϱ� ���ؼ� ���Ǵ� Ŭ�����Դϴ�.
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
	/* �ھ���� ���� ���ư��� ���� ���̾����� �����Դ�. */
	/* ���� ��Ҹ� ���� �޼ҵ� */
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

	// �� �ܿ��� �������� ���ϵ��� �ϱ� ���ؼ� private�� ����
private:
	Layer();

public:
	// Safe_Delete_VecList�� ���ؼ�
	~Layer();

private:
	class Scene* m_pScene;	// �� ���̾ ���� ��� ������ ������.
	string		m_strTag;
	// ������Ʈ�� ������ ���� ����� �׷� �����̴�.
	// �׷��� ������ �߰� ����,������ ������ ���� �߻��� ���̴�.
	// �׷� �κ��� vector���ٴ� list�� ������.
	list<class Object*> m_ObjList;
	bool		m_bEnable;	// ���̾� Ȱ��ȭ
	bool		m_bLife;	// �츮�°� ���̴°�.
};

