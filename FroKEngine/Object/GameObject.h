#pragma once

#include "Object.h"

class GameObject : 
	public Object
{
private : 
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
	class Material* Mat = nullptr;
	MeshGeometry* pGeometry = nullptr;

	// �⺻ ���� ���� ����
	D3D12_PRIMITIVE_TOPOLOGY primitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced �Ű�������
	UINT nIdxCnt = 0;
	UINT nStartIdxLocation = 0;
	int nBaseVertexLocation = 0;

protected:
	GameObject();
	GameObject(const GameObject& ref);
	virtual ~GameObject();

public : 
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual void Collision(float fDeltaTime);
	virtual void Render(float fDeltaTime);
	virtual Object* Clone() = 0;
};

