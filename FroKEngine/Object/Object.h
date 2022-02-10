#pragma once

#include "../Ref.h"

using namespace std;

class CScene;
class CLayer;

// ���۷��� ī���ʹ� Ư�� Ŭ������ �̿��ؼ�
// �̸� �����մϴ�.
class Object : public Ref
{
public : 
	void SetScene(class Scene* pScene)
	{
		this->m_pScene = pScene;
	}

	void SetLayer(class Layer* pLayer)
	{
		this->m_pLayer = pLayer;
	}

	class Scene* GetScene() const 
	{
		return m_pScene;
	}

	class Layer* GetLayer() const
	{
		return m_pLayer;
	}

protected:
	Object();
	Object(const Object& ref);
	virtual ~Object();

	/* ���� ��Ҹ� ���� �޼ҵ� */
public:
	virtual bool Init() = 0;
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual void Collision(float fDeltaTime);
	virtual void Render(float fDeltaTime);
		virtual Object* Clone() = 0;
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile);

public:
	void SaveFromPath(const char* pFileName, const string& strPathKey = DATA_PATH);
	void SaveFromFullPath(const char* pFullPath);
	void LoadFromPath(const char* pFileName, const string& strPathKey = DATA_PATH);
	void LoadFromFullPath(const char* pFullPath);


public:
	// ������ �پ��� Ÿ���� ������Ʈ�� ����� ���� 
	// ���ø� Ÿ���� �޼ҵ��̴�.
	template <typename T>
	static T* CreateObj(const string& strTag,
		class Layer* pLayer = nullptr);
	// ������ ������Ÿ���� ���縦 �����Ѵ�.
	// ���ø����� ���� ������ ����.
	static Object* CreateCloneObj(const string& strProtoKey,
		const string& strTag,
		SCENE_CREATE sc,
		class Layer* pLayer = nullptr);

// ������Ʈ �ڷ� �ٷ�� 
public:
	static void AddObj(Object* pObj);
	static Object* FindObj(const string& strTag);
	static void EraseObj();	// ��ü
	static void EraseObj(Object* pObj);
	static void EraseObj(const string& strTag);

protected:
	// �ڱⰡ ���� ���� ���̾ �˰� �Ѵ�.
	class Scene* m_pScene;
	class Layer* m_pLayer;

protected:
	// ���� ������ �������� ��ü�� ���� ������ �����ϴ� ���� ����̴�.
	// �� ����� ���� ���� �󿡼��� ��ü�� ��ġ, ����, ũ�⸦ �����Ѵ�.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	// �ؽ�ó�� ��ġ
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	bool					m_bPhysics;
	// �߷� �ð���
	float					m_fGravityTime;

protected:
	friend class Scene;

private:
	// �����Ǵ� ��� ������Ʈ���� ���⼭ ����ȴ�.
	// �� ���̾ ��ġ�Ǹ鼭 ���ÿ� ���⿡ ���´ٴ� ���̴�.
	// (���� ��ġ��)
	static list<Object*>	m_ObjList;
};