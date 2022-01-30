#pragma once

#include "../Ref.h"

using namespace std;

class CScene;
class CLayer;

// ���۷��� ī���ʹ� Ư�� Ŭ������ �̿��ؼ�
// �̸� �����մϴ�.
class Object : public Ref
{

protected:
	Object();
	Object(const Object& ref);
	virtual ~Object();

	/* ���� ��Ҹ� ���� �޼ҵ� */
public:
	virtual bool Init() = 0;
	virtual void Input(float fDeltaTime);
	virtual int Update(float fDeltaTime);
	virtual int LateUpdate(float fDeltaTime);
	virtual void Collision(float fDeltaTime);
	virtual void Render(float fDeltaTime);
		virtual Object* Clone() = 0;
	virtual void Save(FILE* pFile);
	virtual void Load(FILE* pFile);

public:
	void SaveFromPath(const char* pFileName, const string& strPathKey = DATA_PATH);
	void SaveFromFullPath(const char* pFullPath);
	void LoadFromPath(const char* pFileName, const string& strPathKey = DATA_PATH);
	void LoadFromFullPath(const char* pFullPath);

protected:
	// ���� ������ �������� ��ü�� ���� ������ �����ϴ� ���� ����̴�.
	// �� ����� ���� ���� �󿡼��� ��ü�� ��ġ, ����, ũ�⸦ �����Ѵ�.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	// �ؽ�ó�� ��ġ
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	bool					m_bPhysics;
	// �߷� �ð���
	float					m_fGravityTime;

protected:
	// �ڱⰡ ���� ���� ���̾ �˰� �Ѵ�.
	class CScene* m_pScene;
	class CLayer* m_pLayer;

protected:
	friend class CScene;

private:
	// �����Ǵ� ��� ������Ʈ���� ���⼭ ����ȴ�.
	// �� ���̾ ��ġ�Ǹ鼭 ���ÿ� ���⿡ ���´ٴ� ���̴�.
	// (���� ��ġ��)
	static list<Object*>	m_ObjList;
};