#pragma once
#include "Object.h"

class Texture : public Object
{
public:
	Texture();
	virtual ~Texture();

	void Init(const wstring& path);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandle() { return _srvHandle; }

public:
	void CreateTexture(const wstring& path);
	void CreateView();

private:
	ScratchImage			 		_image;
	ComPtr<ID3D12Resource>			_tex2D;

	// 뷰를 2개 가지고 있다.
	// 텍스처는 한 번 로딩하고 계속 쓰기 때문에
	// 뷰를 하나씩 들고 있어도 된다.
	ComPtr<ID3D12DescriptorHeap>	_srvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE		_srvHandle;
};

