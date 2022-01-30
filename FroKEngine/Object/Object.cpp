#include "Object.h"

void Object::Input(float fDeltaTime)
{
}

int Object::Update(float fDeltaTime)
{
    return 0;
}

int Object::LateUpdate(float fDeltaTime)
{
    return 0;
}

void Object::Collision(float fDeltaTime)
{
}

void Object::Render(float fDeltaTime)
{
}

Object* Object::Clone()
{
    return nullptr;
}

void Object::Save(FILE* pFile)
{
}

void Object::Load(FILE* pFile)
{
}

void Object::SaveFromPath(const char* pFileName, const string& strPathKey)
{
}

void Object::SaveFromFullPath(const char* pFullPath)
{
}

void Object::LoadFromPath(const char* pFileName, const string& strPathKey)
{
}

void Object::LoadFromFullPath(const char* pFullPath)
{
}
