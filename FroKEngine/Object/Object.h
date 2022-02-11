#pragma once

#include "../Ref.h"
#include "../Collision/Collider.h"

using namespace std;

class CScene;
class CLayer;

// 레퍼런스 카운터는 특수 클래스를 이용해서
// 이를 관리합니다.
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

	/* 게임 요소를 위한 메소드 */
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


public:
	void SaveFromPath(const char* pFileName, const string& strPathKey = DATA_PATH);
	void SaveFromFullPath(const char* pFullPath);
	void LoadFromPath(const char* pFileName, const string& strPathKey = DATA_PATH);
	void LoadFromFullPath(const char* pFullPath);


public:
	// 굉장히 다양한 타입의 오브젝트를 만들기 위한 
	// 템플릿 타입의 메소드이다.
	template <typename T>
	static T* CreateObj(const string& strTag,
		class Layer* pLayer = nullptr);
	// 생성된 프로토타입의 복사를 생성한다.
	// 템플릿으로 만들 이유도 없다.
	static Object* CreateCloneObj(const string& strProtoKey,
		const string& strTag,
		SCENE_CREATE sc,
		class Layer* pLayer = nullptr);

// 오브젝트 자료 다루기 
public:
	static void AddObj(Object* pObj);
	static Object* FindObj(const string& strTag);
	static void EraseObj();	// 전체
	static void EraseObj(Object* pObj);
	static void EraseObj(const string& strTag);

public : 
	XMFLOAT3 GetPos() const
	{
		return m_tPos;
	}

protected:
	// 자기가 속한 장면과 레이어를 알게 한다.
	class Scene* m_pScene;
	class Layer* m_pLayer;

public : 
	XMFLOAT4X4 GetWorldMatrix()
	{
		return m_world;
	}

	void SetWorldMatrix(XMFLOAT4X4 world)
	{
		m_world = world;
	}

	XMFLOAT4X4 GetTexTransform()
	{
		return m_texTransform;
	}

	void SetTexTransform(XMFLOAT4X4 texTransform)
	{
		m_texTransform = texTransform;
	}

	void SetTexTransform(XMMATRIX texTransform)
	{
		XMStoreFloat4x4(&m_texTransform, texTransform);
	}

	int GetObjCBIdx() const
	{
		return m_objCBIdx;
	}

	void SetObjCBIdx(int idx)
	{
		m_objCBIdx = idx;
	}

	D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveType() const
	{
		return m_primitiveType;
	}

	void SetPrimitiveType(D3D12_PRIMITIVE_TOPOLOGY type)
	{
		m_primitiveType = type;
	}

	UINT GetIdxCnt() const
	{
		return m_nIdxCnt;
	}

	void SetIdxCnt(UINT cnt)
	{
		m_nIdxCnt = cnt;
	}

	UINT GetStartIdxLocation() const
	{
		return m_nStartIdxLocation;
	}

	void SetStartIdxLocation(UINT location)
	{
		m_nStartIdxLocation = location;
	}

	UINT GetBaseVertexLocation() const
	{
		return m_nBaseVertexLocation;
	}

	void SetBaseVertexLocation(int location)
	{
		m_nBaseVertexLocation = location;
	}

	int GetFrameDirty() const
	{
		return m_nFramesDirty;
	}

	void SetFrameDirty(int dirty)
	{
		m_nFramesDirty = dirty;
	}

protected:
	// 세계 공간을 기준으로 물체의 로컬 공간을 서술하는 세계 행렬이다.
	// 이 행렬은 세계 공간 상에서의 물체의 위치, 방향, 크기를 결정한다.
	XMFLOAT4X4 m_world = MathHelper::Identity4x4();

	XMFLOAT3 m_tPos;

	// 텍스처의 위치
	XMFLOAT4X4 m_texTransform = MathHelper::Identity4x4();

	// 물체의 자료가 변해서 상수 버퍼를 갱신해야 하는 지의 여부를 뜻하는 더티 플래그이다.
	// FrameResource마다 물체의 cbuffer가 있으므로, FrameResource마다 갱신을 적용해야 한다.
	// 따라서 물체의 자료를 수정할 때에는 반드시
	// NumFrameDirty = gNumFrameResources로 설정해야한다.
	// 그래야 각각의 프레임 자원이 갱신된다.
	int m_nFramesDirty = gNumFrameResource;

	// 이 렌더 아이템의 물체 상수 버퍼에 해당하는 GPU 상수 버퍼의 색인
	UINT m_objCBIdx = -1;

	// 기본 도형 위상 구조
	D3D12_PRIMITIVE_TOPOLOGY m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced 매개변수들
	UINT	m_nIdxCnt = 0;
	UINT	m_nStartIdxLocation = 0;
	int		m_nBaseVertexLocation = 0;

	// 충돌체에 대한 정보
	list<Collider*>		m_ColliderList;

protected:
	friend class Scene;

private:
	// 생성되는 모든 오브젝트들은 여기서 저장된다.
	// 즉 레이어에 배치되면서 동시에 여기에 들어온다는 뜻이다.
	// (실제 배치됨)
	static list<Object*>	m_ObjList;
};