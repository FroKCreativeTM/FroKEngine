#pragma once

#include "Scene.h"

class WaveSimulatorScene : 
	public Scene
{
private:
	friend class SceneManager; // 씬 매니저만 이 클래스에 접근 가능하다.

private:
	WaveSimulatorScene();
	~WaveSimulatorScene();

public:
	virtual bool Init();
};

