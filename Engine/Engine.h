#pragma once

#include "Device.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "RootSignature.h"
#include "Mesh.h"
#include "Shader.h"
#include "ConstantBuffer.h"
#include "TableDescriptorHeap.h"
#include "Texture.h"
#include "DepthStencilBuffer.h"

#include "Material.h"

class Engine
{
public : 
	void Init(const WindowInfo& info);

public : 
	void Update();
	void LateUpdate();
	void ResizeWindow(int32 width, int32 height);

public :
	shared_ptr<Device> GetDevice() { return _device; }
	shared_ptr<CommandQueue> GetCmdQueue() { return _cmdQueue; }
	shared_ptr<SwapChain> GetSwapChain() { return _swapChain; }
	shared_ptr<RootSignature> GetRootSignature() { return _rootSignature; }
	shared_ptr<TableDescriptorHeap> GetTableDescHeap() { return _tableDescHeap; }
	shared_ptr<DepthStencilBuffer> GetDepthStencilBuffer() { return _depthStencilBuffer; }

	shared_ptr<ConstantBuffer> GetConstantBuffer(CONSTANT_BUFFER_TYPE type) { return _constantBuffers[static_cast<uint8>(type)]; }

	const WindowInfo& GetWindow() const { return _window; }

private :
	void Render();
	void RenderBegin();
	void RenderEnd();

private : 
	void ShowFps();
	void CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count);

private : 
	WindowInfo _window;
	D3D12_VIEWPORT _viewport = {};
	D3D12_RECT _scissorRect = {};

	shared_ptr<Device> _device;
	shared_ptr<CommandQueue> _cmdQueue;
	shared_ptr<SwapChain> _swapChain;
	shared_ptr<RootSignature> _rootSignature;
	shared_ptr<TableDescriptorHeap> _tableDescHeap;
	shared_ptr<DepthStencilBuffer> _depthStencilBuffer;

	vector<shared_ptr<ConstantBuffer>> _constantBuffers;
};