#pragma once

#include "../Ref.h"

using namespace std;

class CScene;
class CLayer;

// 레퍼런스 카운터는 특수 클래스를 이용해서
// 이를 관리합니다.
class Object : public Ref
{

protected:
	Object();
	Object(const Object& ref);
	virtual ~Object();

	/* 게임 요소를 위한 메소드 */
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
	// 세계 공간을 기준으로 물체의 로컬 공간을 서술하는 세계 행렬이다.
	// 이 행렬은 세계 공간 상에서의 물체의 위치, 방향, 크기를 결정한다.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	// 텍스처의 위치
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	bool					m_bPhysics;
	// 중력 시간값
	float					m_fGravityTime;

protected:
	// 자기가 속한 장면과 레이어를 알게 한다.
	class CScene* m_pScene;
	class CLayer* m_pLayer;

protected:
	friend class CScene;

private:
	// 생성되는 모든 오브젝트들은 여기서 저장된다.
	// 즉 레이어에 배치되면서 동시에 여기에 들어온다는 뜻이다.
	// (실제 배치됨)
	static list<Object*>	m_ObjList;
};