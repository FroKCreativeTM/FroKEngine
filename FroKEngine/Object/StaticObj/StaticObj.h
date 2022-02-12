#pragma once

#include "../Object.h"

class StaticObj : 
	public Object
{
protected:
	StaticObj();
	StaticObj(const StaticObj& ref);
	virtual ~StaticObj();

public:
	virtual bool Init() = 0;// ������ �־�ߵǴ� ���� �����Լ�;
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual void Collision(float fDeltaTime);
	virtual void Render(ComPtr<ID3D12GraphicsCommandList> commandList, float fDeltaTime);
	virtual StaticObj* Clone() = 0;
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile);
};