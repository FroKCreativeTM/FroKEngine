#pragma once

#include "../Core.h"
#include "Scene.h"
#include "SceneManager.h"
#include "Layer.h"
#include "../Graphics/Camera.h"
#include "../Resource/ResourceManager.h"
#include "../Graphics/UploadBuffer.h"
#include "../Graphics/FrameResource.h"
#include "../Object/StaticObj/MeshObject.h"
#include "../Graphics/GeometryGenerator.h"
#include "../Graphics/Material.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Texture/Texture.h"
#include "../Wave.h"
#include "../TreeSpriteVertex.h"

class WaveSimulatorScene : 
	public Scene
{
private:
	friend class SceneManager; // �� �Ŵ����� �� Ŭ������ ���� �����ϴ�.

public:
	/* ���� ��Ҹ� ���� �޼ҵ� */
	virtual bool Init();

private:
	void OnMouseDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseUp(int x, int y);

private:
	WaveSimulatorScene();
	~WaveSimulatorScene();

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();


	float GetHillsHeight(float x, float z) const
	{
		return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	}

	XMFLOAT3 GetHillsNormal(float x, float z) const
	{
		// n = (-df/dx, 1, -df/dz)
		XMFLOAT3 n(
			-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
			1.0f,
			-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));

		XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
		XMStoreFloat3(&n, unitNormal);

		return n;
	}

	// ������ ���ҽ��� �����Ѵ�.
	void BuildFrameResources();

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
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<MeshObject*>& ritems);

private:
	ComPtr<ID3D12RootSignature>		m_RootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap>	m_CbvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap>	m_SrvHeap = nullptr;

	UINT m_passCbvOffset = 0;

	MeshGeometry* m_BoxGeo = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;
	// ���� ���̴��� �Ѱ��� 
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_TreeSpriteInputLayout;

	// ������ ���ҽ��� �����ϱ� ���� vector
	std::vector<std::unique_ptr<FrameResource>> m_frameResources;
	FrameResource* m_curFrameResource = nullptr;
	int m_nCurFrameResourceIdx = 0;

	UINT m_CbvSrvDescriptorSize = 0;


	// �ĵ��� ���� ������
	Waves* m_Waves;
	MeshObject* m_WaveRenderItem;

	// ���׸����� �����ϱ� ���� ��
	std::unordered_map<std::string, Material*> m_Materials;
	std::vector<Object*> m_allRenderItems;
	bool m_IsWireframe = false;

	// �н� ������� �����ص� ����ü
	PassConstants m_tMainPassCB;

	// ������Ʈ�� ������ �����ϱ� ���� map
	std::unordered_map<std::string, MeshGeometry*> m_Geometries;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;

	// �ؽ�ó�� �����ϱ� ���� ��
	std::unordered_map<std::string, Texture*> m_Textures;

	XMFLOAT4X4	m_World = MathHelper::Identity4x4();
	XMFLOAT3 m_EyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4	m_View = MathHelper::Identity4x4();
	XMFLOAT4X4	m_Proj = MathHelper::Identity4x4();

	// �¾�(���� �ٿ�)�� ��Ʈ���� ����
	float m_fSunTheta = 1.25f * XM_PI;
	float m_fSunPhi = XM_PIDIV4;

	float		m_Theta = 1.5f * XM_PI;
	// float		m_Phi = XM_PIDIV4;
	float		m_Phi = 0.2f * XM_PI;
	float		m_Radius = 15.0f;

	POINT m_LastMousePos;
};

