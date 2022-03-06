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
	Device* GetDevice() { return _device; }
	GraphicsCommandQueue* GetGraphicsCmdQueue() { return _graphicsCmdQueue; }
	ComputeCommandQueue* GetComputeCmdQueue() { return _computeCmdQueue; }
	SwapChain* GetSwapChain() { return _swapChain; }
	RootSignature* GetRootSignature() { return _rootSignature; }
	GraphicsDescriptorHeap* GetGraphicsDescHeap() { return _graphicsDescHeap; }
	ComputeDescriptorHeap* GetComputeDescHeap() { return _computeDescHeap; }

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

	Device* _device;
	GraphicsCommandQueue* _graphicsCmdQueue;
	ComputeCommandQueue* _computeCmdQueue;
	SwapChain* _swapChain;
	RootSignature* _rootSignature;
	GraphicsDescriptorHeap* _graphicsDescHeap;
	ComputeDescriptorHeap* _computeDescHeap;

	ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;

	vector<shared_ptr<ConstantBuffer>> _constantBuffers;
	array<shared_ptr<RenderTargetGroup>, RENDER_TARGET_GROUP_COUNT> _rtGroups;
};