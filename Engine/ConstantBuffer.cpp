#include "pch.h"
#include "ConstantBuffer.h"
#include "Engine.h"

ConstantBuffer::ConstantBuffer()
{
}

ConstantBuffer::~ConstantBuffer()
{
	if (_cbvBuffer)
	{
		if (_cbvBuffer != nullptr)
			_cbvBuffer->Unmap(0, nullptr);

		_cbvBuffer = nullptr;
	}
}

void ConstantBuffer::Init(CBV_REGISTER reg, uint32 size, uint32 count)
{
	_reg = reg;

	// 상수 버퍼는 256 바이트 배수로 만들어야 한다
	// 0 256 512 768
	// 반올림 코드
	// 참고로 ~255는 내림 방식이기 때문에, 255를 더한것이다.
	_elementSize = (size + 255) & ~255;
	_elementCount = count;

	CreateBuffer();
	CreateView();
}

void ConstantBuffer::CreateBuffer()
{
	uint32 bufferSize = _elementSize * _elementCount;
	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	DEVICE->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_cbvBuffer));

	_cbvBuffer->Map(0, nullptr, reinterpret_cast<void**>(&_mappedBuffer));
	// 리소스가 끝날 때까지 매핑을 해제할 필요가 없습니다.
	// 그러나 GPU에서 리소스를 사용하는 동안에는 리소스에 쓰지 않아야 합니다
	// (따라서 동기화 기술을 사용해야 함).
}

void ConstantBuffer::CreateView()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvDesc = {};
	cbvDesc.NumDescriptors = _elementCount;
	cbvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	cbvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	DEVICE->CreateDescriptorHeap(&cbvDesc, IID_PPV_ARGS(&_cbvHeap));

	_cpuHandleBegin = _cbvHeap->GetCPUDescriptorHandleForHeapStart();
	_handleIncrementSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (uint32 i = 0; i < _elementCount; ++i)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = GetCpuHandle(i);

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = _cbvBuffer->GetGPUVirtualAddress() + static_cast<uint64>(_elementSize) * i;
		cbvDesc.SizeInBytes = _elementSize;   // CB size is required to be 256-byte aligned.

		DEVICE->CreateConstantBufferView(&cbvDesc, cbvHandle);
	}
}

void ConstantBuffer::Clear()
{
	_currentIndex = 0;
}

void ConstantBuffer::PushGraphicsData(void* buffer, uint32 size)
{
	assert(_currentIndex < _elementCount);
	assert(_elementSize == ((size + 255) & ~255));

	::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(_currentIndex);
	GEngine->GetGraphicsDescHeap()->SetCBV(cpuHandle, _reg);

	_currentIndex++;
}

void ConstantBuffer::PushComputeData(void* buffer, uint32 size)
{
	assert(_currentIndex < _elementCount);
	assert(_elementSize == ((size + 255) & ~255));

	::memcpy(&_mappedBuffer[_currentIndex * _elementSize], buffer, size);	// 무조건 0번에 

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuHandle(_currentIndex);
	GEngine->GetComputeDescHeap()->SetCBV(cpuHandle, _reg);

	_currentIndex++;
}

void ConstantBuffer::SetGraphicsGloablData(void* buffer, uint32 size)
{
	assert(_elementSize == ((size + 255) & ~255));
	::memcpy(&_mappedBuffer[0], buffer, size);	// 무조건 0번에 
	GRAPHICS_CMD_LIST->SetGraphicsRootConstantBufferView(0, GetGpuVirtualAddress(0));
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGpuVirtualAddress(uint32 index)
{
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = _cbvBuffer->GetGPUVirtualAddress();
	objCBAddress += index * _elementSize;
	return objCBAddress;
}

D3D12_CPU_DESCRIPTOR_HANDLE ConstantBuffer::GetCpuHandle(uint32 index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(_cpuHandleBegin, index * _handleIncrementSize);
}
