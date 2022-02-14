#pragma once

#include "../Game.h"


using namespace std;

class Scene
{
public:
	/* 레이어와 관련된 메소드 */
	class Layer* CreateLayer(const string& strTag,
		int nZOrder = 0);
	class Layer* FindLayer(const string& strTag);

public:
	// 전역 함수이다!!!!(list의 sort caller가 callee로 전역함수를 원한다.)
	static bool LayerSort(class Layer* pL1, class Layer* pL2);

public:
	/* 게임 요소를 위한 메소드 */
	virtual bool Init();
	// 각 씬마다의 고유한 기능을 넣기 위한 메소드다.
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual void Collision(float fDeltaTime);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime);

public:
	// 프로토타입 관련 메소드
	static void ErasePrototype(SCENE_CREATE sc);	// 전체
	static void ErasePrototype(const string& strTag, SCENE_CREATE sc);

	// 프로토타입 제작 메소드
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
	friend class SceneManager;	// 씬 매니저만 이 클래스에 접근 가능하다.

protected:
	Scene();
	virtual ~Scene() = 0;	// 순수 가상함수로!


// 오브젝트 자체를 레이어에 배치할 것이다.
// 레이어는 리스트로 보관하자.
protected:
	// 이 방식을 이용하면 장면이 날아가면 그 장면에 종속된 모든
	// 레이어들도 함께 날아간다.
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
	// 생성할 때만 필요하다.
	static class Object* FindPrototype(const string& strTag, SCENE_CREATE sc);
	static void ChangePrototype();

private : 
	virtual void OnResize();

private:
	// 원본 객체(프로토타입)을 관리
	// 복사할 용도, 
	// 예로들어 오크를 만든다 치면
	// HP 등등의 정보가 있을 것이다.
	// 근데 문제는 모든 몬스터는 정보가 다를 것이다.
	// 그러면 파일에 있는 것을 로딩할텐데 문제는 느려!
	// 그렇기 때문에 여기다가 미리 만들어놓고, 리스폰될 객체들을
	// 만들면 될 것이다.
	static unordered_map<string, class Object*> m_mapPrototype[SC_END];
};