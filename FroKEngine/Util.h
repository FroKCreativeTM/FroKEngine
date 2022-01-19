#pragma once

#include <fstream>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

// Direct3D 관련된 유틸 함수를 넣습니다.
class D3DUtil
{
public : 
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* pDevice,
		ID3D12GraphicsCommandList* pCmdList,
		const void* pInitData,
		UINT byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);

	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// Constant buffers must be a multiple of the minimum hardware
		// allocation size (usually 256 bytes).  So round up to nearest
		// multiple of 256.  We do this by adding 255 and then masking off
		// the lower 2 bytes which store all bits < 256.
		// Example: Suppose byteSize = 300.
		// (300 + 255) & ~255
		// 555 & ~255
		// 0x022B & ~0x00ff
		// 0x022B & 0xff00
		// 0x0200
		// 512
		return (byteSize + 255) & ~255;
	}

	static ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);
};

// 기본 버퍼를 만들어주기 위한 편의 함수입니다.
inline Microsoft::WRL::ComPtr<ID3D12Resource> D3DUtil::CreateDefaultBuffer(ID3D12Device* pDevice, 
	ID3D12GraphicsCommandList* pCmdList, 
	const void* pInitData, 
	UINT byteSize, 
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
	Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// CPU의 메모리 자료를 기본 버퍼로 복사하기 위해 
	// 임시 업로드 힙을 생성한다.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

	// 기본 버퍼에 복사할 자료를 서술한다.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = pInitData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// 기본 버퍼 자원으로의 자료 복사를 요청한다.
	// 보조 함수 UpdateSubresources는 CPU 메모리를
	// 입시 업로드 힙에 복사하고, ID3D12CommandList::CopySubresourceRegion을
	// 이용해서 입시 업로드 힙의 자료를 mBuffer에 복사한다.
	pCmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST));

	UpdateSubresources<1>(pCmdList,
		defaultBuffer.Get(), uploadBuffer.Get(),
		0, 0, 1, &subResourceData);

	pCmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ));

	// Caution!!! 
	// 위의 함수 호출 이후에도 uploadBuffer를 계속 유지해야 한다.
	// 실제로 복사를 수행하는 커맨드 리스트가 실행되지 않았기 때문이다.
	// 복사가 완료된 것이 확실해진 이후에 호출자가 uploadBuffer를 해제해주면 된다.
	return defaultBuffer;
}

inline Microsoft::WRL::ComPtr<ID3DBlob> D3DUtil::CompileShader(const std::wstring& filename, 
	const D3D_SHADER_MACRO* defines, 
	const std::string& entrypoint, 
	const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return byteCode;
}

// 오프라인 컴파일된 셰이더(보통 .cso로 끝난다.)를
// 로딩하기 위한 함수이다.
inline ComPtr<ID3DBlob> D3DUtil::LoadBinary(const std::wstring& filename)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios::end);
	std::ifstream::pos_type size = static_cast<int>(fin.tellg());
	fin.seekg(0, std::ios::beg);

	ComPtr<ID3DBlob> blob;
	ThrowIfFailed(D3DCreateBlob(size, blob.GetAddressOf()));

	fin.read((char*)blob->GetBufferPointer(), size);
	fin.close();

	return blob;
}


// MeshGeometry에서 지오메트리의 하위 범위를 정의합니다.
// 하나의 정점과 인덱스 버퍼에 여러 도형을 저장할 때 사용합니다. 
// 그림 6.3에서 설명한 기술을 구현할 수 있도록 
// 꼭짓점 및 인덱스 버퍼에 있는 지오메트리 저장소의 하위 집합을 그리는 데 필요한 오프셋과 데이터를 제공합니다.
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// 이 서브메시에 의해 정의된 지오메트리의 경계 상자이다.
	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	// Give it a name so we can look it up by name.
	std::string Name;

	// System memory copies.  Use Blobs because the vertex/index format can be generic.
	// It is up to the client to cast appropriately.  
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	// A MeshGeometry may store multiple geometries in one vertex/index buffer.
	// Use this container to define the Submesh geometries so we can draw
	// the Submeshes individually.
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	// We can free this memory after we finish upload to the GPU.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};

// 보통 이런 구조체는 
// HLSL의 순서에 맞춰서 개발을 진행한다.
struct Light
{
	DirectX::XMFLOAT3 Strength;		// 빛의 색상
	float FalloffStart;				// 점광과 점적광에만 쓰인다.
	DirectX::XMFLOAT3 Direction;	// 지향광과 점적광에만 쓰인다.
	float FalloffEnd;				// 점광과 점적광에만 쓰인다.
	DirectX::XMFLOAT3 Position;		// 점광과 점적광에만 쓰인다.
	float SpotPower;				// 점적광에만 쓰인다.
};