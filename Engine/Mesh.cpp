#include "pch.h"
#include "Mesh.h"
#include "Engine.h"
#include "Material.h"

Mesh::Mesh() : Object(OBJECT_TYPE::MESH)
{

}

Mesh::~Mesh()
{

}

void Mesh::Init(vector<Vertex>& vertexBuffer, const vector<uint32>& indexBuffer)
{
	CreateVertexBuffer(vertexBuffer);
	CreateIndexBuffer(indexBuffer);
}

void Mesh::Render()
{
	// �ﰢ�� �⺻ ������ �̿��Ѵ�.
	GRAPHICS_CMD_LIST->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// ������ ���� 0����
	GRAPHICS_CMD_LIST->IASetVertexBuffers(0, 1, &_vertexBufferView); // Slot: (0~15)
	// �ε��� ���۵� �Ѱ�����.
	GRAPHICS_CMD_LIST->IASetIndexBuffer(&_indexBufferView);

	// ������ �����͸� ������.
	GEngine->GetGraphicsDescHeap()->CommitTable();

	// �� �ν��Ͻ��� �׸���. (�ν��Ͻ� ���� �� ��)
	// CMD_LIST->DrawInstanced(_vertexCount, 1, 0, 0);
	GRAPHICS_CMD_LIST->DrawIndexedInstanced(_indexCount, 1, 0, 0, 0);
}

void Mesh::CreateVertexBuffer(const vector<Vertex>& vertexBuffer)
{
	_vertexCount = static_cast<uint32>(vertexBuffer.size());
	uint32 bufferSize = _vertexCount * sizeof(Vertex);

	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	DEVICE->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));

	// Copy the triangle data to the vertex buffer.
	void* vertexDataBuffer = nullptr;
	CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
	_vertexBuffer->Map(0, &readRange, &vertexDataBuffer);
	::memcpy(vertexDataBuffer, &vertexBuffer[0], bufferSize);
	_vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vertexBufferView.StrideInBytes = sizeof(Vertex); // ���� 1�� ũ��
	_vertexBufferView.SizeInBytes = bufferSize; // ������ ũ��	
}

void Mesh::CreateIndexBuffer(const vector<uint32>& indexBuffer)
{
	_indexCount = static_cast<uint32>(indexBuffer.size());
	uint32 bufferSize = _indexCount * sizeof(uint32);

	D3D12_HEAP_PROPERTIES heapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	DEVICE->CreateCommittedResource(
		&heapProperty,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_indexBuffer));

	// �ﰢ�� ���ؽ� ������ ���� 
	void* indexDataBuffer = nullptr;
	CD3DX12_RANGE readRange(0, 0); // �츮�� CPU�� �� ���ҽ����� ���� ������ �����ϴ�.
	// Map�� ���ؼ� ���ؽ� ������ ���۸� ���ؼ� �� ������ �����͸� �о��� ��
	// UnMap�� ���ؼ� �ٽ� ���۸� �ݴ´�.
	_indexBuffer->Map(0, &readRange, &indexDataBuffer);
	::memcpy(indexDataBuffer, &indexBuffer[0], bufferSize);
	_indexBuffer->Unmap(0, nullptr);

	// ���ؽ� ���� �並 �����Ѵ�.
	_indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	_indexBufferView.SizeInBytes = bufferSize; // ������ ũ��	
}
