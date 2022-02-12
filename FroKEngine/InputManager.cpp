#include "InputManager.h"

DEFINITION_SINGLE(InputManager);

InputManager::InputManager() :
	m_pCreateKey(nullptr)
{

}

InputManager::~InputManager()
{
	// CObj::EraseObj(m_pMouse);	// 오브젝트 목록에서 지운 뒤에 SAFE_RELEASE
	// SAFE_RELEASE(m_pMouse);
	Safe_Delete_Map(m_mapKey);
}

bool InputManager::Init(HWND hWnd)
{
	m_hWnd = hWnd;

	AddKey('W', "MoveFront");
	AddKey('S', "MoveBack");
	AddKey('D', "MoveRight");
	AddKey('A', "MoveLeft");
	AddKey(VK_CONTROL, "Fire");
	AddKey(VK_SPACE, "Jump");
	AddKey(VK_ESCAPE, "Escape");
	AddKey('1', "WireFrame");
	AddKey(VK_LBUTTON, "MouseLButton");
	AddKey(VK_RBUTTON, "MouseRButton");

	// 마우스 위치를 얻어오는 함수
	GetCursorPos(&m_tMousePos);

	// 마우스 생성
	// m_pMouse = CObj::CreateObj<CMouse>("Mouse");

	// m_pMouse->SetSize(32, 31);
	// m_pMouse->SetTexture("Mouse0", L"StartBackground.bmp");

	// CAnimation* pAni = m_pMouse->CreateAnimation("MouseAnimation");

	// m_pMouse->AddAnimationClip("MouseIdle", AT_ATLAS, AO_LOOP,
	// 	1.f,
	// 	1, 1, // 4장에 1줄짜리
	// 	0, 0,
	// 	1, 1,
	// 	0.f,
	// 	"MouseIdle", L"Mouse/MouseCursor.bmp");
	// m_pMouse->SetAnimationClipColorKey("MouseIdle", 255, 0, 255);
	// 
	// SAFE_RELEASE(pAni);

	return true;
}

void InputManager::Update(float fDeltaTime)
{
	unordered_map<std::string, PKEYINFO>::iterator iter;
	unordered_map<std::string, PKEYINFO>::iterator iterEnd = m_mapKey.end();

	for (iter = m_mapKey.begin(); iter != iterEnd; ++iter)
	{
		int nPushCount = 0;
		for (size_t i = 0; i < iter->second->vecKey.size(); ++i)
		{
			// 체크하려는 키가 눌렸다면
			if (GetAsyncKeyState(iter->second->vecKey[i]) & 0x8000)
			{
				nPushCount++;
			}
		}

		// 누른 경우
		if (nPushCount == iter->second->vecKey.size())
		{
			// 처음 누른 경우
			if (!iter->second->bDown && !iter->second->bPress)
			{
				iter->second->bDown = true;
				iter->second->bPress = true;
			}
			// 계속 누른 경우
			else if (iter->second->bDown)
			{
				iter->second->bDown = false;
			}
		}
		// 안 누름
		else
		{
			// 누르다가 뗀 경우
			if (iter->second->bDown || iter->second->bPress)
			{
				iter->second->bUp = true;
				iter->second->bDown = false;
				iter->second->bPress = false;
			}
			else if (iter->second->bUp)
			{
				iter->second->bUp = false;
			}
		}
	}

	//m_pMouse->Update(fDeltaTime);
	//m_pMouse->LateUpdate(fDeltaTime);

	// 충돌 매니저에 등록해줘야 충돌 처리가 된다.
	// GET_SINGLE(CCollisionManager)->AddObject(m_pMouse);
}

bool InputManager::KeyDown(const string& strKey) const
{
	PKEYINFO pInfo = FindKey(strKey);
	if (!pInfo)
		return false;
	return pInfo->bDown;
}

bool InputManager::KeyPress(const string& strKey) const
{
	PKEYINFO pInfo = FindKey(strKey);
	if (!pInfo)
		return false;
	return pInfo->bPress;
}

bool InputManager::KeyUp(const string& strKey) const
{
	PKEYINFO pInfo = FindKey(strKey);
	if (!pInfo)
		return false;
	return pInfo->bUp;
}

PKEYINFO InputManager::FindKey(const string& strKey) const
{
	unordered_map<string, PKEYINFO>::const_iterator iter
		= m_mapKey.find(strKey);

	if (iter == m_mapKey.end())
	{
		return nullptr;
	}
	return iter->second;
}