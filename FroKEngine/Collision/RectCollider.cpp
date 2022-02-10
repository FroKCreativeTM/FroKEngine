#include "RectCollider.h"

void RectCollider::SetRect(XMFLOAT3 m_vecMin, XMFLOAT3 m_vecMax)
{
	m_tInfo.m_vecMin = m_vecMin;
	m_tInfo.m_vecMax = m_vecMax;
}

void RectCollider::SetRect(float xmin, float ymin, float zmin,
	float xmax, float ymax, float zmax)
{
	m_tInfo.m_vecMin.x = xmin;
	m_tInfo.m_vecMin.y = ymin;
	m_tInfo.m_vecMin.z = zmin;
	m_tInfo.m_vecMax.x = xmax;
	m_tInfo.m_vecMax.y = ymax;
	m_tInfo.m_vecMax.z = zmax;
}

RectCollider::RectCollider()
{
	m_eColType = CT_RECT;
}

RectCollider::RectCollider(const RectCollider& col)
{
	m_tInfo = col.m_tInfo;
}

RectCollider::~RectCollider()
{
}

bool RectCollider::Init()
{
	return false;
}

void RectCollider::Input(float fDeltaTime)
{
	Collider::Input(fDeltaTime);
}

int RectCollider::Update(float fDeltaTime)
{
	Collider::Update(fDeltaTime);
	return 0;
}

int RectCollider::LateUpdate(float fDeltaTime)
{
	Collider::LateUpdate(fDeltaTime);
	return 0;
}

bool RectCollider::Collision(Collider* pDst)
{
	switch (pDst->GetColliderType())
	{
	case CT_RECT : 
		return CollisionRectToRect(m_tInfo, ((RectCollider*)pDst)->GetInfo());
	}
	return false;
}

void RectCollider::Render(HDC hDC, float fDeltaTime)
{
	Collider::Render(hDC, fDeltaTime);
}

RectCollider* RectCollider::Clone()
{
	return new RectCollider(*this);
}

void RectCollider::Save(FILE* pFile)
{
}

void RectCollider::Load(FILE* pFile)
{
}
