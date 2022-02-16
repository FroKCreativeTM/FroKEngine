#include "pch.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "Engine.h"

CommandQueue::~CommandQueue()
{
	// 이벤트를 정리하자
	::CloseHandle(_fenceEvent);
}

void CommandQueue::Init(ComPtr<ID3D12Device> device, shared_ptr<SwapChain> swapChain)
{
	_swapChain = swapChain;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_cmdQueue));

	// - D3D12_COMMAND_LIST_TYPE_DIRECT : GPU가 직접 실행하는 명령 목록
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAlloc));

	// GPU가 하나인 시스템에서는 0으로
	// DIRECT or BUNDLE
	// Allocator
	// 초기 상태 (그리기 명령은 nullptr 지정)
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&_cmdList));

	// CommandList는 Close / Open 상태가 있는데
	// Open 상태에서 Command를 넣다가 Close한 다음 제출하는 개념
	_cmdList->Close();

	// CreateFence
	// - CPU와 GPU의 동기화 수단으로 쓰인다
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	_fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

void CommandQueue::WaitSync()
{
	// 먼저 펜스 포인트까지 명령을 표시하기 위해 증가시킨다.
	_fenceValue++;

	// 명령 대기열에 명령을 추가하여 새 펜스 포인트를 설정한다.  
	// GPU 타임라인에 있기 때문에 GPU가 
	// 이 Signal() 이전의 모든 명령 처리를 완료할 때까지 새 펜스 포인트가 설정되지 않는다.
	_cmdQueue->Signal(_fence.Get(), _fenceValue);

	// GPU가 이 울타리 지점까지 명령을 완료할 때까지 기다린다.
	if (_fence->GetCompletedValue() < _fenceValue)
	{
		// GPU가 현재 울타리에 도달하면 이벤트가 발생한다.
		_fence->SetEventOnCompletion(_fenceValue, _fenceEvent);

		// GPU가 현재 펜스 이벤트에 도달할 때까지 기다린다.
		::WaitForSingleObject(_fenceEvent, INFINITE);
	}
}

void CommandQueue::RenderBegin(const D3D12_VIEWPORT* vp, const D3D12_RECT* rect)
{
	// 캐패시티가 그대로인 벡터를 비우는 느낌이라 보면 된다.
	_cmdAlloc->Reset();
	_cmdList->Reset(_cmdAlloc.Get(), nullptr);

	// 스왑 체인과 같이 노는 것으로 보면 된다.
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_swapChain->GetBackRTVBuffer().Get(), // (★)
		D3D12_RESOURCE_STATE_PRESENT,				// 화면 출력(★)
		D3D12_RESOURCE_STATE_RENDER_TARGET);		// 외주 결과물(★)
	
	_cmdList->SetGraphicsRootSignature(ROOT_SIGNATURE.Get());
	GEngine->GetCB()->Clear();
	GEngine->GetTableDescHeap()->Clear();

	ID3D12DescriptorHeap* descHeap = GEngine->GetTableDescHeap()->GetDescriptorHeap().Get();
	_cmdList->SetDescriptorHeaps(1, &descHeap);	// CommitTable이 정상 작동

	_cmdList->ResourceBarrier(1, &barrier);

	// 뷰포트와 가위 사각형을 설정한다. 
	// 이것은 명령 목록이 재설정될 때마다 재설정되어야 한다.
	_cmdList->RSSetViewports(1, vp);
	_cmdList->RSSetScissorRects(1, rect);

	// 렌더링할 버퍼를 지정한다..
	D3D12_CPU_DESCRIPTOR_HANDLE backBufferView = _swapChain->GetBackRTV();
	_cmdList->ClearRenderTargetView(backBufferView, Colors::LightSteelBlue, 0, nullptr);
	_cmdList->OMSetRenderTargets(1, &backBufferView, FALSE, nullptr);
}

void CommandQueue::RenderEnd()
{
	D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		_swapChain->GetBackRTVBuffer().Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, // 외주 결과물
		D3D12_RESOURCE_STATE_PRESENT); // 화면 출력

	_cmdList->ResourceBarrier(1, &barrier);
	_cmdList->Close();

	// 커맨드 리스트 수행
	ID3D12CommandList* cmdListArr[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(_countof(cmdListArr), cmdListArr);

	_swapChain->Present();

	// 프레임 커맨드가 끝날때까지 기다린다.
	// 이 대기는 비효율적이며 단순성을 위해 수행된다.
	// 나중에 프레임당 기다릴 필요가 없도록 렌더링 코드를 구성하는 방법을 보여줄 것이다.
	WaitSync();

	_swapChain->SwapIndex();
}
