#pragma once

#include "Game.h"

using namespace std;

typedef struct _tagKeyInfo
{
    string			strName;
    // unsigned long
    // 정수나 char 타입이면 push
    vector<DWORD>	vecKey;
    bool			bDown;
    bool			bPress;
    bool			bUp;

    _tagKeyInfo() :
        bDown(false),
        bPress(false),
        bUp(false)
    {}
}KEYINFO, * PKEYINFO;

// 현재 키가 누른 상태인가
// 현재 게임 루프가 동작하는 동안 키를 누르고 있는 상태인가를 저장한다.
class InputManager
{
public:
	bool Init(HWND hWnd);
	void Update(float fDeltaTime);
	bool KeyDown(const string& strKey) const;
	bool KeyPress(const string& strKey) const;
	bool KeyUp(const string& strKey) const;

public:
	template <typename T>
	bool AddKey(const T& data)
	{
		// 처음에 NULL로 되어있다.
		if (!m_pCreateKey)
		{
			m_pCreateKey = new KEYINFO;
		}

		// 템플릿 식별 번호를 만들어서 저장한다.
		const char* pTType = typeid(T).name();

		// 정수나 char인 경우
		if (strcmp(pTType, "char") == 0 ||
			strcmp(pTType, "int") == 0)
		{
			m_pCreateKey->vecKey.push_back((DWORD)data);
		}
		// map의 key값이다.
		else
		{
			m_pCreateKey->strName = data;
			m_mapKey.insert(make_pair(m_pCreateKey->strName, m_pCreateKey));
		}

		return true;
	}

	// template ... Types
	// 가변 템플릿
	// 무조건 재귀함수를 써야...
	template <typename T, typename ... Types>
	bool AddKey(const T& data, const Types& ...arg)
	{
		// 처음에 NULL로 되어있다.
		if (!m_pCreateKey)
		{
			m_pCreateKey = new KEYINFO;
		}

		// 템플릿 식별 번호를 만들어서 저장한다.
		const char* pTType = typeid(T).name();

		// 정수나 char인 경우
		if (strcmp(pTType, "char") == 0 ||
			strcmp(pTType, "int") == 0)
		{
			m_pCreateKey->vecKey.push_back((DWORD)data);
		}
		// map의 key값이다.
		else
		{
			m_pCreateKey->strName = data;
			m_mapKey.insert(make_pair(m_pCreateKey->strName, m_pCreateKey));
		}

		// 재귀로 돌린다.
		AddKey(arg...);

		// 재귀용 변수를 다시 초기화
		if (m_pCreateKey)
		{
			m_pCreateKey = nullptr;
		}

		return true;
	}

	//class CMouse* GetMouse() const
	//{
	//	return m_pMouse;
	//}

	//POSITION  GetMouseClientPos() const;
	//POSITION  GetMouseWorldPos() const;
	//POSITION  GetMouseMove() const;	// 얼마나 마우스가 움직였는가.

private:
	PKEYINFO FindKey(const string& strKey) const;

private:
	HWND		m_hWnd;
	// 이 키를 누르면 호출되는 함수 목록을 매칭할 것이다.
	unordered_map<string, PKEYINFO>	m_mapKey;
	// 재귀 함수 돌리기용
	PKEYINFO	m_pCreateKey;

private:
	// 마우스에 대한 정보
	POINT				m_tMousePos;
	POINT				m_tMouseMove;

	DECLARE_SINGLE(InputManager)
};

