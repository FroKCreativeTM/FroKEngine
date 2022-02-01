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

	// 이 부분은 레벨들을 레이어화하면 해결될 부분인 것 같다.
	void LoadTexture();
	void BuildDescriptorHeaps();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildWavesGeometryBuffers();
	void BuildMaterials();
	void BuildLandGeometry();
	void BuildBoxGeometry();
	void BuildTreeSpritesGeometry();
	void BuildRenderItems();
	void BuildPSO();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<class RenderItem*>& ritems);

public:
	virtual bool Init();

	//virtual void Input(float fDeltaTime);
	//virtual int Update(float fDeltaTime);
	//virtual int LateUpdate(float fDeltaTime);
	//virtual void Collision(float fDeltaTime);
	//virtual void Render(float fDeltaTime);
};

