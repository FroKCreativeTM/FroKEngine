#pragma once

#include "../Game.h"

using namespace DirectX;

enum class RenderLayer : int
{
	Opaque = 0,		// ������ü
	Count
};

// �ϳ��� ��ü�� �׸��� �� �ʿ��� �Ű��������� ��� Ŭ�����̴�.
// �� Ŭ������ ��ü���� ������ ���� ���α׷����� �޶��� ���̴�.
class RenderItem
{
public : 
	RenderItem() = default;

	// ���� ������ �������� ��ü�� ���� ������ �������ϴ� ���� ����̴�.
	// �� ����� ���� ���� �󿡼��� ��ü�� ��ġ, ����, ũ�⸦ �����Ѵ�.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	// ��ü�� �ڷᰡ ���ؼ� ��� ���۸� �����ؾ� �ϴ� ���� ���θ� ���ϴ� ��Ƽ �÷����̴�.
	// FrameResource���� ��ü�� cbuffer�� �����Ƿ�, FrameResource���� ������ �����ؾ� �Ѵ�.
	// ���� ��ü�� �ڷḦ ������ ������ �ݵ��
	// NumFrameDirty = gNumFrameResources�� �����ؾ��Ѵ�.
	// �׷��� ������ ������ �ڿ��� ���ŵȴ�.
	int nFramesDirty = gNumFrameResource;

	// �� ���� �������� ��ü ��� ���ۿ� �ش��ϴ� GPU ��� ������ ����
	UINT objCBIdx = -1;

	// �� ���� �׸� ������ ���� ������ ���׸����� �����Ѵ�. 
	// ���� ������ �׸��� ���� ���� ������ ������ �� ������ �����϶�.
	Material* Mat = nullptr;
	MeshGeometry* pGeometry = nullptr;

	// �⺻ ���� ���� ����
	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced �Ű�������
	UINT nIdxCnt = 0;
	UINT nStartIdxLocation = 0;
	int nBaseVertexLocation = 0;
};