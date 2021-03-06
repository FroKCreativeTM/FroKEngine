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
	Engine();
	~Engine();
	Engine(const Engine& rhs) = delete;
	Engine& operator=(const Engine& rhs) = delete;

public : 
	void Init(const WindowInfo& info);
	void Update();

	void OnResize(const WindowInfo& info);

public :
	Device* GetDevice() { return _device; }
	GraphicsCommandQueue* GetGraphicsCmdQueue() { return _graphicsCmdQueue; }
	ComputeCommandQueue* GetComputeCmdQueue() { return _computeCmdQueue; }
	SwapChain* GetSwapChain() { return _swapChain; }
	RootSignature* GetRootSignature() { return _rootSignature; }
	GraphicsDescriptorHeap* GetGraphicsDescHeap() { return _graphicsDescHeap; }
	ComputeDescriptorHeap* GetComputeDescHeap() { return _computeDescHeap; }

	ConstantBuffer* GetConstantBuffer(CONSTANT_BUFFER_TYPE type) { return _constantBuffers[static_cast<uint8>(type)]; }
	RenderTargetGroup* GetRTGroup(RENDER_TARGET_GROUP_TYPE type) { return _rtGroups[static_cast<uint8>(type)]; }

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

	vector<ConstantBuffer*> _constantBuffers;
	array<RenderTargetGroup*, RENDER_TARGET_GROUP_COUNT> _rtGroups;
};