#pragma once

// 다양한 렌더 타겟을 2D로 띄울 것이기 때문에 포함한다.
#include "Texture.h"

enum class RENDER_TARGET_GROUP_TYPE : uint8
{
	SWAP_CHAIN, // BACK_BUFFER, FRONT_BUFFER
	SHADOW, // SHADOW
	G_BUFFER, // POSITION, NORMAL, COLOR
	LIGHTING, // DIFFUSE LIGHT, SPECULAR LIGHT	
	IMGUI, // IMGUI
	END,
};

enum
{
	RENDER_TARGET_SHADOW_GROUP_MEMBER_COUNT = 1,
	RENDER_TARGET_G_BUFFER_GROUP_MEMBER_COUNT = 3,
	RENDER_TARGET_LIGHTING_GROUP_MEMBER_COUNT = 2,
	RENDER_TARGET_GROUP_COUNT = static_cast<uint8>(RENDER_TARGET_GROUP_TYPE::END)
};

struct RenderTarget
{
	shared_ptr<Texture> target;
	float clearColor[4];
};

// 유니티에서는 Multiple-Render-Target(MRT)이라 지칭한다.
class RenderTargetGroup
{
public:
	void Create(RENDER_TARGET_GROUP_TYPE groupType, vector<RenderTarget>& rtVec, shared_ptr<Texture> dsTexture);

	void OMSetRenderTargets(uint32 count, uint32 offset);
	void OMSetRenderTargets();

	void ClearRenderTargetView(uint32 index);
	void ClearImGuiRenderTargetView(uint32 index);
	void ClearRenderTargetView();

	shared_ptr<Texture> GetRTTexture(uint32 index) { return _rtVec[index].target; }
	shared_ptr<Texture> GetDSTexture() { return _dsTexture; }

	void WaitTargetToResource();
	void WaitResourceToTarget();

private:
	RENDER_TARGET_GROUP_TYPE		_groupType;
	vector<RenderTarget>			_rtVec;
	uint32							_rtCount;
	shared_ptr<Texture>				_dsTexture;
	ComPtr<ID3D12DescriptorHeap>	_rtvHeap;
	ComPtr<ID3D12DescriptorHeap>	_srvHeap;

private:
	uint32							_rtvHeapSize;
	D3D12_CPU_DESCRIPTOR_HANDLE		_rtvHeapBegin;
	D3D12_CPU_DESCRIPTOR_HANDLE		_dsvHeapBegin;

private:
	D3D12_RESOURCE_BARRIER			_targetToResource[8];
	D3D12_RESOURCE_BARRIER			_resourceToTarget[8];

};

