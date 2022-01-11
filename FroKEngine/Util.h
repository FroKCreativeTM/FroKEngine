#pragma once

// Direct3D 관련된 유틸 함수를 넣습니다.
class D3DUtil
{
public : 
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* pDevice,
		ID3D12GraphicsCommandList* pCmdList,
		const void* pInitData,
		UINT byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);
};

// 기본 버퍼를 만들어주기 위한 편의 함수입니다.
Microsoft::WRL::ComPtr<ID3D12Resource> D3DUtil::CreateDefaultBuffer(ID3D12Device* pDevice, 
	ID3D12GraphicsCommandList* pCmdList, 
	const void* pInitData, 
	UINT byteSize, 
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// CPU의 메모리 자료를 기본 버퍼로 복사하기 위해 
	// 임시 업로드 힙을 생성한다.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

	// 기본 버퍼에 복사할 자료를 서술한다.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = pInitData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// 기본 버퍼 자원으로의 자료 복사를 요청한다.
	// 보조 함수 UpdateSubresources는 CPU 메모리를
	// 입시 업로드 힙에 복사하고, ID3D12CommandList::CopySubresourceRegion을
	// 이용해서 입시 업로드 힙의 자료를 mBuffer에 복사한다.
	pCmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST));

	UpdateSubresources<1>(pCmdList,
		defaultBuffer.Get(), uploadBuffer.Get(),
		0, 0, 1, &subResourceData);

	pCmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ));

	// Caution!!! 
	// 위의 함수 호출 이후에도 uploadBuffer를 계속 유지해야 한다.
	// 실제로 복사를 수행하는 커맨드 리스트가 실행되지 않았기 때문이다.
	// 복사가 완료된 것이 확실해진 이후에 호출자가 uploadBuffer를 해제해주면 된다.
	return defaultBuffer;
}