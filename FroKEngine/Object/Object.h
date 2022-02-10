#pragma once

#include "../Ref.h"

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
		virtual Object* Clone() = 0;
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile);

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

protected:
	// 자기가 속한 장면과 레이어를 알게 한다.
	class Scene* m_pScene;
	class Layer* m_pLayer;

protected:
	// 세계 공간을 기준으로 물체의 로컬 공간을 서술하는 세계 행렬이다.
	// 이 행렬은 세계 공간 상에서의 물체의 위치, 방향, 크기를 결정한다.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	// 텍스처의 위치
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	bool					m_bPhysics;
	// 중력 시간값
	float					m_fGravityTime;

protected:
	friend class Scene;

private:
	// 생성되는 모든 오브젝트들은 여기서 저장된다.
	// 즉 레이어에 배치되면서 동시에 여기에 들어온다는 뜻이다.
	// (실제 배치됨)
	static list<Object*>	m_ObjList;
};