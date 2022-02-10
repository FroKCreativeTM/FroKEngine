#pragma once

#include "Collider.h"

using namespace DirectX;

class RectCollider : 
	public Collider
{
public:
	/* 게임 요소를 위한 메소드 */
	virtual bool Init();
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual bool Collision(Collider* pDst);
	virtual void Render(HDC hDC, float fDeltaTime);
	virtual RectCollider* Clone();
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile);

public:
	void SetRect(XMFLOAT3 m_vecMin, XMFLOAT3 m_vecMax);
	void SetRect(float xmin, float ymin, float zmin,
		float xmax, float ymax, float zmax);

	AABB GetInfo() const
	{
		return m_tInfo;
	}

protected:
	RectCollider();
	RectCollider(const RectCollider& col);
	virtual ~RectCollider();

private:
	friend class Object;

private:
	AABB m_tInfo;
};

