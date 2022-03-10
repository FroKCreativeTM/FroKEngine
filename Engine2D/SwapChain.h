#pragma once

class SwapChain
{
public :
	void Init(const WindowInfo& info, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);

	void OnResize(const WindowInfo& info, ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);

	void Present();
	void SwapIndex();

	ComPtr<IDXGISwapChain> GetSwapChain() { return _swapChain; }
	uint8 GetBackBufferIndex() { return _backBufferIndex; }

	DXGI_SWAP_CHAIN_DESC GetDescription() { return _desc; }

private : 
	void CreateSwapChain(const WindowInfo& info, ComPtr<IDXGIFactory> dxgi, ComPtr<ID3D12CommandQueue> cmdQueue);

private :
	ComPtr<IDXGISwapChain>	_swapChain;
	DXGI_SWAP_CHAIN_DESC	_desc;

	uint32					_backBufferIndex = 0;
};

