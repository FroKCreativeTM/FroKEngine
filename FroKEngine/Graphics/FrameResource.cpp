#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT maxInstanceCount, UINT nObjectCnt)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(pCmdListAlloc.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, nPassCnt, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, nObjectCnt, true);
	pInstanceBuffer = new UploadBuffer<InstanceData>(pDevice, maxInstanceCount, false);
}

FrameResource::~FrameResource()
{
	SAFE_DELETE(pInstanceBuffer);
}
