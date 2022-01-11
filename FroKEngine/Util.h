#pragma once

// Direct3D ���õ� ��ƿ �Լ��� �ֽ��ϴ�.
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

// �⺻ ���۸� ������ֱ� ���� ���� �Լ��Դϴ�.
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

	// CPU�� �޸� �ڷḦ �⺻ ���۷� �����ϱ� ���� 
	// �ӽ� ���ε� ���� �����Ѵ�.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

	// �⺻ ���ۿ� ������ �ڷḦ �����Ѵ�.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = pInitData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// �⺻ ���� �ڿ������� �ڷ� ���縦 ��û�Ѵ�.
	// ���� �Լ� UpdateSubresources�� CPU �޸𸮸�
	// �Խ� ���ε� ���� �����ϰ�, ID3D12CommandList::CopySubresourceRegion��
	// �̿��ؼ� �Խ� ���ε� ���� �ڷḦ mBuffer�� �����Ѵ�.
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
	// ���� �Լ� ȣ�� ���Ŀ��� uploadBuffer�� ��� �����ؾ� �Ѵ�.
	// ������ ���縦 �����ϴ� Ŀ�ǵ� ����Ʈ�� ������� �ʾұ� �����̴�.
	// ���簡 �Ϸ�� ���� Ȯ������ ���Ŀ� ȣ���ڰ� uploadBuffer�� �������ָ� �ȴ�.
	return defaultBuffer;
}