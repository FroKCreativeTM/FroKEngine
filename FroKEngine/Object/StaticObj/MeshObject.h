#pragma once
#include "StaticObj.h"
#include "../../Graphics/Material.h"
#include "MeshGeometry.h"

class MeshObject :
    public StaticObj
{
public:
	virtual bool Init();
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual void Collision(float fDeltaTime);
	virtual void Render(HDC hDC, float fDeltaTime);
	virtual MeshObject* Clone();
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile);

public :
	void SetMaterial(Material* mat)
	{
		m_Mat = mat;
	}

	Material* GetMaterial() const
	{
		return m_Mat;
	}

	void SetGeometry(MeshGeometry* geo)
	{
		m_pGeometry = geo;
	}

	MeshGeometry* GetGeometry() const
	{
		return m_pGeometry;
	}

private : 
	// 이 렌더 항목에 연관된 기하 구조및 마테리얼을 연결한다. 
	// 여러 렌더가 항목이 같은 기하 구조를 참조할 수 있음을 주의하라.
	Material* m_Mat = nullptr;
	MeshGeometry* m_pGeometry = nullptr;
};

