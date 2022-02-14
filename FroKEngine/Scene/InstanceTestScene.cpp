#include "InstanceTestScene.h"
#include "../Object/Object.h""
#include "../Graphics/Camera.h"
#include "../Resource/ResourceManager.h"

bool InstanceTestScene::Init()
{
    if (!Scene::Init())
    {
        return false;
    }

    // ���� Ŀ�ǵ� ����Ʈ�� �ʱ�ȭ �Ѵ�.
    ThrowIfFailed(GET_SINGLE(Core)->GetCommandList()->Reset(GET_SINGLE(Core)->GetDirectCmdListAlloc().Get(), nullptr));

    // �� �� �������� �������� ���� ũ�⸦ �����ɴϴ�. 
    // �̰��� �ϵ��� ���� �ٸ��Ƿ� �� ������ �����ؾ� �մϴ�.
    m_CbvSrvDescriptorSize = GET_SINGLE(Core)->GetDevice()
        ->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    GET_SINGLE(Camera)->SetPosition(0.0f, 2.0f, -15.0f);

    LoadTextures();
    BuildRootSignature();
    BuildDescriptorHeaps();
    BuildShadersAndInputLayout();
    BuildSkullGeometry();
    BuildMaterials();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();

    // �ʱ�ȭ ����� �����մϴ�.
    ThrowIfFailed(GET_SINGLE(Core)->GetCommandList()->Close());
    ID3D12CommandList* cmdsLists[] = { GET_SINGLE(Core)->GetCommandList().Get() };
    GET_SINGLE(Core)->GetCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // �ʱ�ȭ�� �Ϸ�� ������ ��ٸ��ϴ�.
    GET_SINGLE(Core)->FlushCommandQueue();

    return true;
}

void InstanceTestScene::Input(float fDeltaTime)
{
}

int InstanceTestScene::Update(float fDeltaTime)
{
	return 0;
}

void InstanceTestScene::Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime)
{
	GET_SINGLE(Camera)->SetLens(0.25f * MathHelper::Pi, GET_SINGLE(Core)->AspectRatio(), 1.0f, 1000.0f);
	BoundingFrustum::CreateFromMatrix(mCamFrustum, GET_SINGLE(Camera)->GetProj());
}

void InstanceTestScene::OnResize()
{
}

void InstanceTestScene::OnKeyboardInput(float fDeltaTime)
{
}

void InstanceTestScene::AnimateMaterials(float fDeltaTime)
{
}

void InstanceTestScene::UpdateInstanceData(float fDeltaTime)
{
}

void InstanceTestScene::UpdateMaterialBuffer(float fDeltaTime)
{
}

void InstanceTestScene::UpdateMainPassCB(float fDeltaTime)
{
}

void InstanceTestScene::LoadTextures()
{
    GET_SINGLE(ResourceManager)->LoadTexture("grassTex", L"Graphics/Texture/Datas/grass.dds");
    GET_SINGLE(ResourceManager)->LoadTexture("waterTex", L"Graphics/Texture/Datas/water1.dds");
    GET_SINGLE(ResourceManager)->LoadTexture("fenceTex", L"Graphics/Texture/Datas/WireFence.dds");
    GET_SINGLE(ResourceManager)->LoadTexture("treeArrayTex", L"Graphics/Texture/Datas/treeArray2.dds");
}

void InstanceTestScene::BuildRootSignature()
{
}

void InstanceTestScene::BuildDescriptorHeaps()
{
}

void InstanceTestScene::BuildShadersAndInputLayout()
{
}

void InstanceTestScene::BuildSkullGeometry()
{
}

void InstanceTestScene::BuildPSOs()
{
}

void InstanceTestScene::BuildFrameResources()
{
}

void InstanceTestScene::BuildMaterials()
{
}

void InstanceTestScene::BuildRenderItems()
{
}

void InstanceTestScene::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<Object*>& ritems)
{
}
