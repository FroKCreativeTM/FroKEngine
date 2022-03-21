#include "pch.h"
#include "Game.h"
#include "Engine.h"
#include "SceneManager.h"
#include "TestScene.h"

void Game::Init(const WindowInfo& info)
{
	GEngine->Init(info);

	TestScene* testScene = new TestScene();
	GET_SINGLE(SceneManager)->LoadScene((Scene*)testScene);
}

void Game::Update()
{
	GEngine->Update();
}