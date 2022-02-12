#include "SceneManager.h"
#include "Scene.h"
#include "WaveSimulatorScene.h"

DEFINITION_SINGLE(SceneManager)

// m_pScene : ���� ���� ��Ÿ����.
SceneManager::SceneManager() :
	m_pScene(nullptr),
	m_pNextScene(nullptr)
{

}

SceneManager::~SceneManager()
{
	SAFE_DELETE(m_pScene);
}

// ���� �����ڷν� �� �ϵ��� �ش�.
bool SceneManager::Init()
{
	// �⺻ ����� ������ �ϳ� �����д�.
	CreateScene<WaveSimulatorScene>(SC_CURRENT);

	return true;
}

void SceneManager::Input(float fDeltaTime)
{
	// ���� ���� �Է��� �޴´�.
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