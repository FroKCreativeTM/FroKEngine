#pragma once
#include "Object.h"

class Texture : public Object
{
public:
	Texture();
	virtual ~Texture();

	virtual void Load(const wstring& path) override;

public:
	// 0부터 다 만들어줌
	void Create(DXGI_FORMAT format, uint32 width, uint32 height,
		const D3D12_HEAP_PROPERTIES& heapProperty, D3D12_HEAP_FLAGS heapFlags,
		D3D12_RESOURCE_FLAGS resFlags, Vec4 clearColor = Vec4());

	// 리소스가 있는 상태에서 텍스처를 만듬(ex.swap chain)
	void CreateFromResource(ComPtr<ID3D12Resource> tex2D);

public :
	ComPtr<ID3D12Resource> GetTex2D() { return _tex2D; }
	ComPtr<ID3D12DescriptorHeap> GetSRV() { return _srvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetRTV() { return _rtvHeap; }
	ComPtr<ID3D12DescriptorHeap> GetDSV() { return _dsvHeap; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() { return _srvHeapBegin; }

public:

private:
	ScratchImage			 		_image;
	ComPtr<ID3D12Resource>			_tex2D;

	// 뷰를 2개 가지고 있다.
	// 텍스처는 한 번 로딩하고 계속 쓰기 때문에
	// 뷰를 하나씩 들고 있어도 된다.
	ComPtr<ID3D12DescriptorHeap>	_srvHeap;	// 셰이더 리소스용
	ComPtr<ID3D12DescriptorHeap>	_rtvHeap;	// 렌더 타겟용
	ComPtr<ID3D12DescriptorHeap>	_dsvHeap;	// 깊이/스텐실용

	D3D12_CPU_DESCRIPTOR_HANDLE		_srvHeapBegin;
};

