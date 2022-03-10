#include "pch.h"
#include "Editor.h"
#include "Engine.h"
#include "SceneManager.h"

void Editor::Init(const WindowInfo& info)
{
	GEngine->Init(info);

	GET_SINGLE(SceneManager)->LoadScene(L"EditorScene");
}

void Editor::Update()
{
	GEngine->Update();
}