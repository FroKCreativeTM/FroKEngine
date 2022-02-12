#include "SceneManager.h"
#include "Scene.h"
#include "WaveSimulatorScene.h"

DEFINITION_SINGLE(SceneManager)

// m_pScene : 현재 씬을 나타낸다.
SceneManager::SceneManager() :
	m_pScene(nullptr),
	m_pNextScene(nullptr)
{

}

SceneManager::~SceneManager()
{
	SAFE_DELETE(m_pScene);
}

// 서브 관리자로써 할 일들을 준다.
bool SceneManager::Init()
{
	// 기본 장면은 무조건 하나 만들어둔다.
	CreateScene<WaveSimulatorScene>(SC_CURRENT);

	return true;
}

void SceneManager::Input(float fDeltaTime)
{
	// 현재 씬의 입력을 받는다.
	m_pScene->Input(fDeltaTime);
}

SCENE_CHANGE SceneManager::Update(float fDeltaTime)
{
	m_pScene->Update(fDeltaTime);
	return ChangeScene();
}

SCENE_CHANGE SceneManager::LateUpdate(float fDeltaTime)
{
	m_pScene->LateUpdate(fDeltaTime);
	return ChangeScene();
}

void SceneManager::Collision(float fDeltaTime)
{
	m_pScene->Collision(fDeltaTime);
}

void SceneManager::Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime)
{
	m_pScene->Render(commandList, fDeltaTime);
}

SCENE_CHANGE SceneManager::ChangeScene()
{
	if (m_pNextScene)
	{
		SAFE_DELETE(m_pScene);
		m_pScene = m_pNextScene;
		m_pNextScene = nullptr;

		// GET_SINGLE(CCollisionManager)->ClearScene();

		m_pScene->SetSceneType(SC_CURRENT);
		Scene::ChangePrototype();

		return SCENE_CHANGE::SC_CHANGE;
	}

	return SCENE_CHANGE::SC_NONE;
}