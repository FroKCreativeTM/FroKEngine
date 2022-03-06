#pragma once

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"
#include "fontawesome/IconsFontAwesome5.h"

static const ImColor white(1.f, 1.f, 1.f, 1.f);
static const ImColor yellow(1.f, 1.f, 0.f, 1.f);
static const ImColor green(0.f, 1.f, 0.f, 1.f);
static const ImColor red(1.f, 0.f, 0.f, 1.f);
static const ImColor blue(0.f, 0.f, 1.f, 1.f);

#define MAX_NUM_IMGUI_IMAGES_PER_FRAME 128

static float f = 0.0f;
static int counter = 0;

class ImGuiManager
{
	DECLARE_SINGLE(ImGuiManager)

public : 
	ImGuiContext* Init(const WindowInfo& info);
	void RenderBegin();
	void RenderEnd();

private : 
	ID3D12DescriptorHeap* _imguiDescriptorHeap;
	CD3DX12_CPU_DESCRIPTOR_HANDLE _startCPUDescriptor;
	CD3DX12_GPU_DESCRIPTOR_HANDLE _startGPUDescriptor;
	uint32 _descriptorHandleIncrementSize;
	uint32 _numImagesThisFrame;

};

