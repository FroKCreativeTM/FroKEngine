#pragma once

#include "../Core.h"
#include "../Graphics/FrameResource.h"
#include "Scene.h"

class InstanceTestScene :
    public Scene
{
private:
    friend class SceneManager; // �� �Ŵ����� �� Ŭ������ ���� �����ϴ�.

public:
	/* ���� ��Ҹ� ���� �޼ҵ� */
	virtual bool Init();

	// ���� �ð� ��� ó����.
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	// ������Ʈ�� ���� ���·� (�浹 ����) ��ó���� �ʿ��� ���
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime);

private:
	virtual void OnResize() override;

	void AnimateMaterials(float fDeltaTime);
	void UpdateInstanceData(float fDeltaTime);
	void UpdateMaterialBuffer(float fDeltaTime);
	void UpdateMainPassCB(float fDeltaTime);

	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildSkullGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<Object*>& ritems);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:
	std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
	FrameResource* m_CurrFrameResource = nullptr;
	int m_CurrFrameResourceIndex = 0;

	UINT m_CbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, class MeshGeometry*> m_Geometries;
	std::unordered_map<std::string, class Material*> m_Materials;
	std::unordered_map<std::string, class Texture*> m_Textures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> m_Shaders;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> m_PSOs;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayout;

	// List of all the render items.
	std::vector<Object*> m_vecAllRenderObject;

	// Render items divided by PSO.
	std::vector<Object*> mOpaqueRitems;

	bool mFrustumCullingEnabled = true;

	BoundingFrustum mCamFrustum;
	PassConstants mMainPassCB;

	POINT m_LastMousePos;
};

