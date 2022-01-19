#pragma once

#include <fstream>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

// Direct3D ���õ� ��ƿ �Լ��� �ֽ��ϴ�.
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

// �⺻ ���۸� ������ֱ� ���� ���� �Լ��Դϴ�.
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

	// CPU�� �޸� �ڷḦ �⺻ ���۷� �����ϱ� ���� 
	// �ӽ� ���ε� ���� �����Ѵ�.
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

	// �⺻ ���ۿ� ������ �ڷḦ �����Ѵ�.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = pInitData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// �⺻ ���� �ڿ������� �ڷ� ���縦 ��û�Ѵ�.
	// ���� �Լ� UpdateSubresources�� CPU �޸𸮸�
	// �Խ� ���ε� ���� �����ϰ�, ID3D12CommandList::CopySubresourceRegion��
	// �̿��ؼ� �Խ� ���ε� ���� �ڷḦ mBuffer�� �����Ѵ�.
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
	// ���� �Լ� ȣ�� ���Ŀ��� uploadBuffer�� ��� �����ؾ� �Ѵ�.
	// ������ ���縦 �����ϴ� Ŀ�ǵ� ����Ʈ�� ������� �ʾұ� �����̴�.
	// ���簡 �Ϸ�� ���� Ȯ������ ���Ŀ� ȣ���ڰ� uploadBuffer�� �������ָ� �ȴ�.
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

// �������� �����ϵ� ���̴�(���� .cso�� ������.)��
// �ε��ϱ� ���� �Լ��̴�.
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


// MeshGeometry���� ������Ʈ���� ���� ������ �����մϴ�.
// �ϳ��� ������ �ε��� ���ۿ� ���� ������ ������ �� ����մϴ�. 
// �׸� 6.3���� ������ ����� ������ �� �ֵ��� 
// ������ �� �ε��� ���ۿ� �ִ� ������Ʈ�� ������� ���� ������ �׸��� �� �ʿ��� �����°� �����͸� �����մϴ�.
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// �� ����޽ÿ� ���� ���ǵ� ������Ʈ���� ��� �����̴�.
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

// ���� �̷� ����ü�� 
// HLSL�� ������ ���缭 ������ �����Ѵ�.
struct Light
{
	DirectX::XMFLOAT3 Strength;		// ���� ����
	float FalloffStart;				// ������ ���������� ���δ�.
	DirectX::XMFLOAT3 Direction;	// ���Ɽ�� ���������� ���δ�.
	float FalloffEnd;				// ������ ���������� ���δ�.
	DirectX::XMFLOAT3 Position;		// ������ ���������� ���δ�.
	float SpotPower;				// ���������� ���δ�.
};