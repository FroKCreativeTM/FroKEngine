#include "MeshObject.h"
#include "../Object.h"
#include "../../Scene/Scene.h"

bool MeshObject::Init()
{
    return false;
}

void MeshObject::Input(float fDeltaTime)
{
}

int MeshObject::Update(float fDeltaTime)
{
    return 0;
}

int MeshObject::LateUpdate(float fDeltaTime)
{
    return 0;
}

void MeshObject::Collision(float fDeltaTime)
{
}

void MeshObject::Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime)
{
	//commandList->IASetVertexBuffers(0, 1, &GetGeometry()->VertexBufferView());
	//commandList->IASetIndexBuffer(&GetGeometry()->IndexBufferView());
	//commandList->IASetPrimitiveTopology(GetPrimitiveType());

	//CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_SrvHeap->GetGPUDescriptorHandleForHeapStart());
	//tex.Offset(GetMaterial()->nDiffuseSrvHeapIdx, m_CbvSrvDescriptorSize);

	//D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() +
	//	GetObjCBIdx() * objCBByteSize;
	//D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() +
	//	GetMaterial()->nMatCBIdx * matCBByteSize;

	//commandList->SetGraphicsRootDescriptorTable(0, tex);
	//commandList->SetGraphicsRootConstantBufferView(1, objCBAddress);
	//commandList->SetGraphicsRootConstantBufferView(3, matCBAddress);

	//commandList->DrawIndexedInstanced(GetIdxCnt(), 1, GetStartIdxLocation(), GetBaseVertexLocation(), 0);
}

MeshObject* MeshObject::Clone()
{
    return nullptr;
}

void MeshObject::Save(FILE* pFile)
{
}

void MeshObject::Load(FILE* pFile)
{
}

MeshObject::~MeshObject()
{
}
