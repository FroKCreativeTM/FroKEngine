#pragma once


// 어떤 버퍼를 사용할 것인가 또는 어떤 레지스터를 사용할 것인가 등의
// CPU가 GPU에 계약하는 느낌의 클래스이다.
class RootSignature
{
public:
	void Init();

	ComPtr<ID3D12RootSignature>	GetGraphicsRootSignature() { return  _graphicsRootSignature; }
	ComPtr<ID3D12RootSignature>	GetComputeRootSignature() { return  _computeRootSignature; }

private :
	void CreateGraphicsRootSignature();
	void CreateComputeRootSignature();

private:
	ComPtr<ID3D12RootSignature>	_graphicsRootSignature;
	ComPtr<ID3D12RootSignature>	_computeRootSignature;
	D3D12_STATIC_SAMPLER_DESC	_samplerDesc;
};

