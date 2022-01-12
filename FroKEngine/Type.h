#pragma once

// 갠적으로 내가 선언한 타입을 저장합니다.

// 해상도 관련 정보를 저장합니다.
typedef struct _tagResolution
{
	unsigned int nWidth;
	unsigned int nHeight;

	_tagResolution() :
		nWidth(0), nHeight(0) {}

	_tagResolution(int x, int y) :
		nWidth(x), nHeight(y) {}
}RESOLUTION, * PRESOLUTION;


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