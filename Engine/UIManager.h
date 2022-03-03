#pragma once

enum
{
	NUM_BACK_BUFFERS = 2,
};

class Device;
class SwapChain;
class GraphicsDescriptorHeap;

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64                  FenceValue;
};

// ImGui
class UIManager
{
	DECLARE_SINGLE(UIManager)

public:
	void Init(HWND hwnd, shared_ptr<Device> device, shared_ptr<SwapChain> swapchain, shared_ptr<GraphicsDescriptorHeap> graphicsDescHeap);

	void Update();
	void Render();

private : 
	ComPtr<ID3D12DescriptorHeap> _pd3dRtvDescHeap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE  _mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

	UINT                         _frameIndex = 0;
};

