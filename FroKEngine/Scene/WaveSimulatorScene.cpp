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

	/* ���̾ ã�ƾ��Ѵ�. */
	Layer* pLayer = FindLayer("Default");



	return true;
}
