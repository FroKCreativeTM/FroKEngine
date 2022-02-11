#pragma once

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

struct  MeshGeometry
{
	std::string Name;

	// 시스템 메모리 복사본. 꼭짓점/색인 형식이 일반적일 수 있으므로 Blob을 사용합니다.
	// 적절하게 캐스팅하는 것은 클라이언트의 몫입니다.
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// 버퍼 정볻ㄹ
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	// MeshGeometry는 하나의 정점/인덱스 버퍼에 여러 도형을 저장할 수 있습니다.
	// 이 컨테이너를 사용하여 Submesh 지오메트리를 정의하면 Submesh를 개별적으로 그릴 수 있습니다.
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

	// GPU에 업로드를 완료한 후 이 메모리를 해제할 수 있습니다.
	void DisposeUploaders()
	{
		VertexBufferUploader = nullptr;
		IndexBufferUploader = nullptr;
	}
};

