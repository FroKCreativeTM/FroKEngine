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
	// CommandQueue : DX12�� ����
	// ���ָ� ��û�� ��, �ϳ��� ��û�ϸ� ��ȿ����
	// [���� ���]�� �ϰ��� �������� ����ߴٰ� �� �濡 ��û�ϴ� ��
	ComPtr<ID3D12CommandQueue>			_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_cmdList;

	// ���ҽ� ������
	ComPtr<ID3D12CommandAllocator>		_resCmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_resCmdList;	

	// Fence : ��Ÿ��(?)
	// CPU / GPU ����ȭ�� ���� ������ ����
	// Queue�� ������� �� ������ ��ٸ��ڴ�
	ComPtr<ID3D12Fence>					_fence;
	// �� ��° �ϰ��� ���������� ��ٸ��°�
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
	// [���� ���]�� �ϰ��� �������� ����ߴٰ� �� �濡 ��û�ϴ� ��
	ComPtr<ID3D12CommandQueue>			_cmdQueue;
	ComPtr<ID3D12CommandAllocator>		_cmdAlloc;
	ComPtr<ID3D12GraphicsCommandList>	_cmdList;

	ComPtr<ID3D12Fence>					_fence;
	uint32								_fenceValue = 0;
	HANDLE								_fenceEvent = INVALID_HANDLE_VALUE;

	SwapChain*		_swapChain;
	DescriptorHeap*	_descHeap;
};