#pragma once

#include "Scene.h"

class WaveSimulatorScene : 
	public Scene
{
private:
	friend class SceneManager; // �� �Ŵ����� �� Ŭ������ ���� �����ϴ�.

private:
	WaveSimulatorScene();
	~WaveSimulatorScene();

public:
	virtual bool Init();
};

