#pragma once
#include "Object.h"

class Material;

// 정점으로 이루어진 물체
class Mesh : public Object
{
public:
	Mesh();
	virtual ~Mesh();

	void Init(vector<Vertex>& vertexBuffer, const vector<uint32>& indexBuffer);
	void Render(uint32 instanceCount = 1);

private : 
	void CreateVertexBuffer(const vector<Vertex>& vertexBuffer);
	void CreateIndexBuffer(const vector<uint32>& indexBuffer);

private:
	ComPtr<ID3D12Resource>		_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW	_vertexBufferView = {};
	uint32						_vertexCount = 0;

	ComPtr<ID3D12Resource>		_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW		_indexBufferView = {};
	uint32						_indexCount = 0;
};

