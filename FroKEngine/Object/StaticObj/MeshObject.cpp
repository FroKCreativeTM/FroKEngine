#include "MeshObject.h"
#include "../Object.h"
#include "../../Scene/Scene.h"

bool MeshObject::Init()
{
	// StaticObj::Init();

    return true;
}

void MeshObject::Input(float fDeltaTime)
{
	StaticObj::Input(fDeltaTime);
}

int MeshObject::Update(float fDeltaTime)
{
	StaticObj::Update(fDeltaTime);
    return 0;
}

int MeshObject::LateUpdate(float fDeltaTime)
{
	StaticObj::LateUpdate(fDeltaTime);
    return 0;
}

void MeshObject::Collision(float fDeltaTime)
{
	StaticObj::Collision(fDeltaTime);
}

void MeshObject::Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime)
{
	StaticObj::Render(commandList, fDeltaTime);

	commandList->IASetVertexBuffers(0, 1, &GetGeometry()->VertexBufferView());
	commandList->IASetIndexBuffer(&GetGeometry()->IndexBufferView());
	commandList->IASetPrimitiveTopology(GetPrimitiveType());
}

MeshObject* MeshObject::Clone()
{
	return new MeshObject(*this);
}

void MeshObject::Save(FILE* pFile)
{
}

void MeshObject::Load(FILE* pFile)
{
}

MeshObject::MeshObject()
{
}

MeshObject::MeshObject(const MeshObject& pMesh)
{
}

MeshObject::~MeshObject()
{
}
