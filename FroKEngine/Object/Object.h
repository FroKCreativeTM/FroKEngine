#pragma once

#include "../Ref.h"
#include "../Collision/Collider.h"

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
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile); 
	virtual Object* Clone() = 0;

public:
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

public : 
	XMFLOAT3 GetPos() const
	{
		return m_tPos;
	}

protected:
	// �ڱⰡ ���� ���� ���̾ �˰� �Ѵ�.
	class Scene* m_pScene;
	class Layer* m_pLayer;

protected:
	// ���� ������ �������� ��ü�� ���� ������ �����ϴ� ���� ����̴�.
	// �� ����� ���� ���� �󿡼��� ��ü�� ��ġ, ����, ũ�⸦ �����Ѵ�.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT3 m_tPos;

	// �ؽ�ó�� ��ġ
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	bool					m_bPhysics;
	// �߷� �ð���
	float					m_fGravityTime;

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
	friend class Scene;

private:
	// �����Ǵ� ��� ������Ʈ���� ���⼭ ����ȴ�.
	// �� ���̾ ��ġ�Ǹ鼭 ���ÿ� ���⿡ ���´ٴ� ���̴�.
	// (���� ��ġ��)
	static list<Object*>	m_ObjList;
};