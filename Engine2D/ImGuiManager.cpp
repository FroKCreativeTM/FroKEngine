#include "pch.h"
#include "ImGuiManager.h"
#include "Engine.h"
#include "CommandQueue.h"

DEFINITION_SINGLE(ImGuiManager)

static void setStyle()
{
	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };


	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameBorderSize = 1.f;
	style.FramePadding = ImVec2(5.f, 2.f);
}

ImGuiManager::ImGuiManager()
{

}

ImGuiManager::~ImGuiManager()
{

}

ImGuiContext* ImGuiManager::Init(const WindowInfo& info)
{
	IMGUI_CHECKVERSION();
	auto imguiContext = ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.f;
		style.Colors[ImGuiCol_WindowBg].w = 1.f;
	}

	setStyle();

	// io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", 18.f);

	// Merge in icons.
	static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFontConfig iconsConfig;
	iconsConfig.MergeMode = true;
	iconsConfig.PixelSnapH = true;
	// io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/icons/" FONT_ICON_FILE_NAME_FAS, 16.f, &iconsConfig, iconsRanges);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT * MAX_NUM_IMGUI_IMAGES_PER_FRAME + 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	DEVICE->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_imguiDescriptorHeap));

	_startCPUDescriptor = _imguiDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	_startGPUDescriptor = _imguiDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	_descriptorHandleIncrementSize = DEVICE->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	DXGI_FORMAT screenFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	ImGui_ImplWin32_Init(info.hwnd);
	ImGui_ImplDX12_Init(DEVICE.Get(), SWAP_CHAIN_BUFFER_COUNT,
		screenFormat, _imguiDescriptorHeap,
		_startCPUDescriptor,
		_startGPUDescriptor);

	// {
	// 	iconsTexture = loadTextureFromFile("assets/icons/icons_ui.svg", image_load_flags_gen_mips_on_cpu | image_load_flags_cache_to_dds);
	// 
	// 	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(startCPUDescriptor, 1, descriptorHandleIncrementSize);
	// 	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(startGPUDescriptor, 1, descriptorHandleIncrementSize);
	// 	dxContext.device->CopyDescriptorsSimple(1, cpuHandle, iconsTexture->defaultSRV.cpuHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	// 
	// 	iconsTextureID = (ImTextureID)gpuHandle.ptr;
	// }

	return imguiContext;
}

void ImGuiManager::OnResize(const WindowInfo& info)
{
	ImVec2 avail_size = ImGui::GetContentRegionAvail();
}

void ImGuiManager::RenderBegin()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	{
		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}

void ImGuiManager::RenderEnd()
{
	ImGui::Render();
	if (GRAPHICS_CMD_LIST.Get())
	{
		GRAPHICS_CMD_LIST->SetDescriptorHeaps(1, &_imguiDescriptorHeap);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GRAPHICS_CMD_LIST.Get());
	}

	auto& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		if (GRAPHICS_CMD_LIST.Get())
		{
			ImGui::RenderPlatformWindowsDefault(0, GRAPHICS_CMD_LIST.Get());
		}
	}

	_numImagesThisFrame = 0;
}
