#pragma once

class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

	void Init(uint32 size, uint32 count);

	void Clear();
	void PushData(int32 rootParamIndex, void* buffer, uint32 size);

	D3D12_GPU_VIRTUAL_ADDRESS	GetGpuVirtualAddress(uint32 index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle();

private:
	void CreateBuffer();
	void CreateView();

private:
	ComPtr<ID3D12Resource>			_cbvBuffer;
	BYTE*							_mappedBuffer = nullptr;
	uint32							_elementSize = 0;
	uint32							_elementCount = 0;

	// 상수 버퍼 뷰에 대한 정보가 담긴 디스크립터 힙
	ComPtr<ID3D12DescriptorHeap>	_cbvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE		_cpuHandleBegin = {};
	uint32							_handleInIncrementSize = 0;	// 각 핸들 간의 간격

	uint32							_currentIndex = 0;
};

