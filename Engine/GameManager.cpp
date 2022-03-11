#include "pch.h"
#include "GameManager.h"

#include "GameObject.h"
#include "SceneManager.h"
#include "Scene.h"

#include "Input.h"

DEFINITION_SINGLE(GameManager)

GameManager::GameManager()
{

}

GameManager::~GameManager()
{

}

void GameManager::Update()
{
	if (GET_SINGLE(Input)->GetButtonDown(KEY_TYPE::ESC))
	{
		HandlingMenu();
	}
}

void GameManager::HandlingMenu()
{
	auto& gameObjects = GET_SINGLE(SceneManager)->GetActiveScene()->GetGameObjects();

	for (auto iter : gameObjects)
	{
		if (iter->GetName() == L"MainMenu")
		{
			bool b = iter->GetLife();
			iter->SetLife(!b);
		}
	}
}
