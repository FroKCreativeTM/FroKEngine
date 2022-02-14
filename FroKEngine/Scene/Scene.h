#pragma once

#include "../Game.h"


using namespace std;

class Scene
{
public:
	/* ���̾�� ���õ� �޼ҵ� */
	class Layer* CreateLayer(const string& strTag,
		int nZOrder = 0);
	class Layer* FindLayer(const string& strTag);

public:
	// ���� �Լ��̴�!!!!(list�� sort caller�� callee�� �����Լ��� ���Ѵ�.)
	static bool LayerSort(class Layer* pL1, class Layer* pL2);

public:
	/* ���� ��Ҹ� ���� �޼ҵ� */
	virtual bool Init();
	// �� �������� ������ ����� �ֱ� ���� �޼ҵ��.
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual void Collision(float fDeltaTime);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime);

public:
	// ������Ÿ�� ���� �޼ҵ�
	static void ErasePrototype(SCENE_CREATE sc);	// ��ü
	static void ErasePrototype(const string& strTag, SCENE_CREATE sc);

	// ������Ÿ�� ���� �޼ҵ�
	template<typename T>
	T* CreatePrototype(const string& strTag, SCENE_CREATE sc)
	{
		T* pObj = new T;

		pObj->SetTag(strTag);

		if (!pObj->Init())
		{
			SAFE_RELEASE(pObj);
			return nullptr;
		}

		pObj->AddRef();
		m_mapPrototype[sc].insert(make_pair(strTag, pObj));

		return pObj;
	}

protected:
	friend class SceneManager;	// �� �Ŵ����� �� Ŭ������ ���� �����ϴ�.

protected:
	Scene();
	virtual ~Scene() = 0;	// ���� �����Լ���!


// ������Ʈ ��ü�� ���̾ ��ġ�� ���̴�.
// ���̾�� ����Ʈ�� ��������.
protected:
	// �� ����� �̿��ϸ� ����� ���ư��� �� ��鿡 ���ӵ� ���
	// ���̾�鵵 �Բ� ���ư���.
	list<class Layer*> m_LayerList;
	SCENE_CREATE		m_eSceneType;

public:
	void SetSceneType(SCENE_CREATE eType)
	{
		m_eSceneType = eType;
	}

	SCENE_CREATE GetSceneType() const
	{
		return m_eSceneType;
	}

public:
	// ������ ���� �ʿ��ϴ�.
	static class Object* FindPrototype(const string& strTag, SCENE_CREATE sc);
	static void ChangePrototype();

private : 
	virtual void OnResize();

private:
	// ���� ��ü(������Ÿ��)�� ����
	// ������ �뵵, 
	// ���ε�� ��ũ�� ����� ġ��
	// HP ����� ������ ���� ���̴�.
	// �ٵ� ������ ��� ���ʹ� ������ �ٸ� ���̴�.
	// �׷��� ���Ͽ� �ִ� ���� �ε����ٵ� ������ ����!
	// �׷��� ������ ����ٰ� �̸� ��������, �������� ��ü����
	// ����� �� ���̴�.
	static unordered_map<string, class Object*> m_mapPrototype[SC_END];
};