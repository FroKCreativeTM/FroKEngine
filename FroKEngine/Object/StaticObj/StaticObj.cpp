#include "StaticObj.h"

StaticObj::StaticObj()
{
}

StaticObj::StaticObj(const StaticObj& ref) : 
    Object(ref)
{
}

StaticObj::~StaticObj()
{
}

void StaticObj::Input(float fDeltaTime)
{
	Object::Input(fDeltaTime);
}

int StaticObj::Update(float fDeltaTime)
{
	Object::Update(fDeltaTime);
	return 0;
}

int StaticObj::LateUpdate(float fDeltaTime)
{
	Object::LateUpdate(fDeltaTime);
	return 0;
}

void StaticObj::Collision(float fDeltaTime)
{
	Object::Collision(fDeltaTime);
}

void StaticObj::Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime)
{
	Object::Render(commandList, fDeltaTime);
}

void StaticObj::Save(FILE* pFile)
{
	Object::Save(pFile);
}

void StaticObj::Load(FILE* pFile)
{
	Object::Load(pFile);
}
