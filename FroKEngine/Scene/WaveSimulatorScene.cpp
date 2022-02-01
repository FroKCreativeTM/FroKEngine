#include "WaveSimulatorScene.h"
#include "Layer.h"
#include "../Graphics/Camera.h"

WaveSimulatorScene::WaveSimulatorScene()
{
}

WaveSimulatorScene::~WaveSimulatorScene()
{
}

bool WaveSimulatorScene::Init()
{
	if (!Scene::Init())
	{
		return false;
	}

	/* 레이어를 찾아야한다. */
	Layer* pLayer = FindLayer("Default");



	return true;
}
