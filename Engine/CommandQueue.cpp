#include "pch.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "Engine.h"

CommandQueue::~CommandQueue()
{
	// �̺�Ʈ�� ��������
	::CloseHandle(_fenceEvent);
}

void CommandQueue::Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain)
{
	_swapChain = swapChain;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_cmdQueue));

	// - D3D12_COMMAND_LIST_TYPE_DIRECT : GPU�� ���� �����ϴ� ��� ���
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAlloc));

	// GPU�� �ϳ��� �ý��ۿ����� 0����
	// DIRECT or BUNDLE
	// Allocator
	// �ʱ� ���� (�׸��� ����� nullptr ����)
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList));

	// CommandList�� Close / Open ���°� �ִµ�
	// Open ���¿��� Command�� �ִٰ� Close�� ���� �����ϴ� ����
	_cmdList->Close();

	// CreateFence
	// - CPU�� GPU�� ����ȭ �������� ���δ�
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void CommandQueue::WaitSync()
{
	// ���� �潺 ����Ʈ���� ����� ǥ���ϱ� ���� ������Ų��.
	_fenceValue++;

	// ��� ��⿭�� ����� �߰��Ͽ� �� �潺 ����Ʈ�� �����Ѵ�.  
	// GPU Ÿ�Ӷ��ο� �ֱ� ������ GPU�� 
	// �� Signal() ������ ��� ��� ó���� �Ϸ��� ������ �� �潺 ����Ʈ�� �������� �ʴ´�.
	_cmdQueue->Signal(_fence.Get(), _fenceValue);

	// GPU�� �� ��Ÿ�� �������� ����� �Ϸ��� ������ ��ٸ���.
	if (_fence->GetCompletedValue() < _fenceValue)
	{
		// GPU�� ���� ��Ÿ���� �����ϸ� �̺�Ʈ�� �߻��Ѵ�.
		_fence->SetEventOnCompletion(_fenceValue, _fenceEvent);

		// GPU�� ���� �潺 �̺�Ʈ�� ������ ������ ��ٸ���.
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

void CommandQueue::RenderBegin(const D3D12_VIEWPORT* vp, const D3D12_RECT* rect)
{
	// ĳ�н�Ƽ�� �״���� ���͸� ���� �����̶� ���� �ȴ�.
	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	// ���� ü�ΰ� ���� ��� ������ ���� �ȴ�.
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_swapChain->GetBackRTVBuffer().Get(), // (��)
		D3D12_RESOURCE_STATE_PRESENT,				// ȭ�� ���(��)
		D3D12_RESOURCE_STATE_RENDER_TARGET);		// ���� �����(��)
	
	_cmdList->SetGraphicsRootSignature(ROOT_SIGNATURE.Get());
	GEngine->GetCB()->Clear();
	GEngine->GetTableDescHeap()->Clear();

	ID3D12DescriptorHeap* descHeap = GEngine->GetTableDescHeap()->GetDescriptorHeap().Get();
	_cmdList->SetDescriptorHeaps(1, &descHeap);	// CommitTable�� ���� �۵�

	_cmdList->ResourceBarrier(1, &barrier);

	// ����Ʈ�� ���� �簢���� �����Ѵ�. 
	// �̰��� ��� ����� �缳���� ������ �缳���Ǿ�� �Ѵ�.
	_cmdList->RSSetViewports(1, vp);
	_cmdList->RSSetScissorRects(1, rect);

	// �������� ���۸� �����Ѵ�..
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = _swapChain->GetBackRTV();
	_cmdList->ClearRenderTargetView(backBufferView, Colors::LightSteelBlue, 0, nullptr);
	_cmdList->OMSetRenderTargets(1, &backBufferView, FALSE, nullptr);
}

void CommandQueue::RenderEnd()
{
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_swapChain->GetBackRTVBuffer().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, // ���� �����
		D3D12_RESOURCE_STATE_PRESENT); // ȭ�� ���

	_cmdList->ResourceBarrier(1, &barrier);
	_cmdList->Close();

	// Ŀ�ǵ� ����Ʈ ����
	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	_swapChain->Present();

	// ������ Ŀ�ǵ尡 ���������� ��ٸ���.
	// �� ���� ��ȿ�����̸� �ܼ����� ���� ����ȴ�.
	// ���߿� �����Ӵ� ��ٸ� �ʿ䰡 ������ ������ �ڵ带 �����ϴ� ����� ������ ���̴�.
	WaitSync();

	_swapChain->SwapIndex();
}
