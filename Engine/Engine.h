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
#include "RenderTargetGroup.h"

#include "Material.h"

class Engine
{
public : 
	void Init(const WindowInfo& info);
	void Update();

public :
	shared_ptr<Device> GetDevice() { return _device; }
	shared_ptr<GraphicsCommandQueue> GetGraphicsCmdQueue() { return _graphicsCmdQueue; }
	shared_ptr<ComputeCommandQueue> GetComputeCmdQueue() { return _computeCmdQueue; }
	shared_ptr<SwapChain> GetSwapChain() { return _swapChain; }
	shared_ptr<RootSignature> GetRootSignature() { return _rootSignature; }
	shared_ptr<GraphicsDescriptorHeap> GetGraphicsDescHeap() { return _graphicsDescHeap; }
	shared_ptr<ComputeDescriptorHeap> GetComputeDescHeap() { return _computeDescHeap; }

	shared_ptr<ConstantBuffer> GetConstantBuffer(CONSTANT_BUFFER_TYPE type) { return _constantBuffers[static_cast<uint8>(type)]; }
	shared_ptr<RenderTargetGroup> GetRTGroup(RENDER_TARGET_GROUP_TYPE type) { return _rtGroups[static_cast<uint8>(type)]; }

	const WindowInfo& GetWindow() { return _window; }

	void ResizeWindow(int32 width, int32 height);

private :
	void Render();
	void RenderBegin();
	void RenderEnd();

private : 
	void ShowFps();
	void CreateConstantBuffer(CBV_REGISTER reg, uint32 bufferSize, uint32 count);
	void CreateRenderTargetGroups();

private : 
	WindowInfo _window;
	D3D12_VIEWPORT _viewport = {};
	D3D12_RECT _scissorRect = {};

	shared_ptr<Device> _device;
	shared_ptr<GraphicsCommandQueue> _graphicsCmdQueue;
	shared_ptr<ComputeCommandQueue> _computeCmdQueue;
	shared_ptr<SwapChain> _swapChain;
	shared_ptr<RootSignature> _rootSignature;
	shared_ptr<GraphicsDescriptorHeap> _graphicsDescHeap;
	shared_ptr<ComputeDescriptorHeap> _computeDescHeap;

	vector<shared_ptr<ConstantBuffer>> _constantBuffers;
	array<shared_ptr<RenderTargetGroup>, RENDER_TARGET_GROUP_COUNT> _rtGroups;
};