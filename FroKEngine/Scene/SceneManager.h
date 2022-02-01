#pragma once

#include "../Game.h"


class SceneManager
{
public : 
	bool Init();

	void Input(float fDeltaTime);
	SCENE_CHANGE Update(float fDeltaTime);
	SCENE_CHANGE LateUpdate(float fDeltaTime);
	void Collision(float fDeltaTime);

	// 디바이스가 필요한 경우 수정할 예정
	void Render(float fDeltaTime); 


public:
	template <typename T>
	T* CreateScene(SCENE_CREATE sc = SC_CURRENT)
	{
		T* pScene = new T;

		pScene->SetSceneType(sc);

		if (!pScene->Init())
		{
			SAFE_DELETE(pScene);
			return nullptr;
		}

		// 장면을 전환한다.
		switch (sc)
		{
		case SC_CURRENT:
			SAFE_DELETE(m_pScene);
			m_pScene = pScene;
			break;
		case SC_NEXT:
			SAFE_DELETE(m_pNextScene);
			m_pNextScene = pScene;
			break;
		}

		return pScene;
	}

public:
	class Scene* GetScene() const
	{
		return m_pScene;
	}

private:
	SCENE_CHANGE ChangeScene();

private:
	class Scene* m_pScene;
	class Scene* m_pNextScene;	// 장면 전환용 장면

	DECLARE_SINGLE(SceneManager)
};