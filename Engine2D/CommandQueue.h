#pragma once

class SwapChain;
class DescriptorHeap;
class GraphicsDescriptorHeap;

// ********************************
// GraphicsCommandQueue
// ********************************

class GraphicsCommandQueue
{
public :
	~GraphicsCommandQueue();

	void Init(ComPtr<ID3D12Device> device, SwapChain* swapChain);
	void WaitSync();

	void RenderBegin();
	void RenderEnd();
	
	void FlushResourceCommandQueue();

	void Reset();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return _cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList>	GetGraphicsCmdList() { return _cmdList; }
	ComPtr<ID3D12GraphicsCommandList>	GetResourceCmdList() { return _resCmdList; }

private: 
	// CommandQueue : DX12에 등장
	// 외주를 요청할 때, 하나씩 요청하면 비효율적
	// [외주 목록]에 일감을 차곡차곡 기록했다가 한 방에 요청하는 것
	ComPtr<ID3D12CommandQueue>			_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_cmdList;

	// 리소스 관리용
	ComPtr<ID3D12CommandAllocator>		_resCmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_resCmdList;	

	// Fence : 울타리(?)
	// CPU / GPU 동기화를 위한 간단한 도구
	// Queue의 결과물이 올 때까지 기다리겠다
	ComPtr<ID3D12Fence>					_fence;
	// 몇 번째 일감이 끝날때까지 기다리는가
	uint32								_fenceValue = 0;
	HANDLE								_fenceEvent = INVALID_HANDLE_VALUE;

	SwapChain*		_swapChain;
	DescriptorHeap*	_descHeap;
};


// ********************************
// ComputeCommandQueue
// ********************************

class ComputeCommandQueue
{
public:
	~ComputeCommandQueue();

	void Init(ComPtr<ID3D12Device> device);
	void WaitSync();

	void FlushComputeCommandQueue();

	void Reset();

	ComPtr<ID3D12CommandQueue> GetCmdQueue() { return _cmdQueue; }
	ComPtr<ID3D12GraphicsCommandList>	GetComputeCmdList() { return _cmdList; }


private:
	// [외주 목록]에 일감을 차곡차곡 기록했다가 한 방에 요청하는 것
	ComPtr<ID3D12CommandQueue>			_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_cmdList;

	ComPtr<ID3D12Fence>					_fence;
	uint32								_fenceValue = 0;
	HANDLE								_fenceEvent = INVALID_HANDLE_VALUE;

	SwapChain*		_swapChain;
	DescriptorHeap*	_descHeap;
};