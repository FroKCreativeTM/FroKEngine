#include "Collider.h"
#include "../Object/GameObject.h"

Collider::Collider()
{
}

Collider::Collider(const Collider& col)
{
	*this = col;
}

Collider::~Collider()
{
	list<Collider*>::iterator iter;
	list<Collider*>::iterator iterEnd = m_CollisionList.end();

	for (iter = m_CollisionList.begin(); iter != iterEnd; ++iter)
	{
		// �� ���� ���� �� ����
		(*iter)->EraseCollsionList(this);
	}
}

void Collider::Input(float fDeltaTime)
{
}

int Collider::Update(float fDeltaTime)
{
	return 0;
}

int Collider::LateUpdate(float fDeltaTime)
{
	return 0;
}

bool Collider::Collision(Collider* pDst)
{
	return false;
}

void Collider::Render(HDC hDC, float fDeltaTime)
{
}

void Collider::Save(FILE* pFile)
{
	// �±� ����(������!!!)
	int nLength = m_strTag.length();

	// �±� ���� ����
	fwrite(&nLength, 4, 1, pFile);

	// ���ڿ� ����
	fwrite(m_strTag.c_str(), 1, nLength, pFile);

	// �浹ü Ÿ�� ����
	fwrite(&m_eColType, 4, 1, pFile);
}

void Collider::Load(FILE* pFile)
{
}

bool Collider::CollisionRectToRect(const AABB& tBox1, const AABB& tBox2)
{
	return(tBox1.m_vecMax.x > tBox2.m_vecMin.x &&
		tBox1.m_vecMin.x < tBox2.m_vecMax.x&&
		tBox1.m_vecMax.y > tBox2.m_vecMin.y &&
		tBox1.m_vecMin.y < tBox2.m_vecMax.y&&
		tBox1.m_vecMax.z > tBox2.m_vecMin.z &&
		tBox1.m_vecMin.z < tBox2.m_vecMax.z);
}
