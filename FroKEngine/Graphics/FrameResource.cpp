#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(pCmdListAlloc.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, nPassCnt, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, nObjectCnt, true);
}

FrameResource::FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt, UINT waveVertCount)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(pCmdListAlloc.GetAddressOf())));

	PassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, nPassCnt, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, nObjectCnt, true);

	WavesVB = std::make_unique<UploadBuffer<Vertex>>(pDevice, waveVertCount, false);
}

FrameResource::FrameResource(ID3D12Device* pDevice, UINT nPassCnt, UINT nObjectCnt, UINT nMaterialCount, UINT waveVertCount)
{
	ThrowIfFailed(pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(pCmdListAlloc.GetAddressOf())));

	//  FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, 1, true);
	PassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, nPassCnt, true);
	MaterialCB = std::make_unique<UploadBuffer<MaterialConstants>>(pDevice, nMaterialCount, true);
	ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, nObjectCnt, true);

	WavesVB = std::make_unique<UploadBuffer<Vertex>>(pDevice, waveVertCount, false);
}

FrameResource::~FrameResource()
{
}
