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

	// �� �κ��� �������� ���̾�ȭ�ϸ� �ذ�� �κ��� �� ����.
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

