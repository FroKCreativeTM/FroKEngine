#include "pch.h"
#include "UIManager.h"
#include "Engine.h"
#include "Device.h"
#include "SwapChain.h"

void UIManager::Init(HWND hwnd, shared_ptr<SwapChain> swapchain, shared_ptr<GraphicsDescriptorHeap> graphicsDescHeap)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    desc.NumDescriptors = NUM_BACK_BUFFERS;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 1;
    if (DEVICE->CreateDescriptorHeap(&desc,
        IID_PPV_ARGS(&_pd3dRtvDescHeap)) != S_OK)
        return;

    SIZE_T rtvDescriptorSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        _mainRenderTargetDescriptor[i] = rtvHandle;
        rtvHandle.ptr += rtvDescriptorSize;
    }


    // ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(DEVICE.Get(), swapchain->GetDescription().BufferCount,
        DXGI_FORMAT_R8G8B8A8_UNORM, graphicsDescHeap->GetDescriptorHeap().Get(),
        graphicsDescHeap->GetDescriptorHeap().Get()->GetCPUDescriptorHandleForHeapStart(),
        graphicsDescHeap->GetDescriptorHeap().Get()->GetGPUDescriptorHandleForHeapStart());
}

void UIManager::Update()
{
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void UIManager::Render()
{
    ImGui::Render();
}