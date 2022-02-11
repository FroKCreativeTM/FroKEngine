#include "MeshObject.h"

bool MeshObject::Init()
{
    return false;
}

void MeshObject::Input(float fDeltaTime)
{
}

int MeshObject::Update(float fDeltaTime)
{
    return 0;
}

int MeshObject::LateUpdate(float fDeltaTime)
{
    return 0;
}

void MeshObject::Collision(float fDeltaTime)
{
}

void MeshObject::Render(HDC hDC, float fDeltaTime)
{
}

MeshObject* MeshObject::Clone()
{
    return nullptr;
}

void MeshObject::Save(FILE* pFile)
{
}

void MeshObject::Load(FILE* pFile)
{
}
