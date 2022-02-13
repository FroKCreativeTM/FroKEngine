#include "InputManager.h"
#include "Core.h"

DEFINITION_SINGLE(InputManager);

InputManager::InputManager() :
	m_pCreateKey(nullptr)
{
	for (size_t i = 0; i < NSInput::KEYS_ARRAY_LEN; i++) {
		m_keysDown[i] = false;
	}

	for (size_t i = 0; i < NSInput::KEYS_ARRAY_LEN; i++) {
		m_keysPressed[i] = false;
	}

	m_newLine = true;
	m_textIn = "";
	m_charIn = 0;
	m_mouseX = 0;
	m_mouseY = 0;
	m_mouseRawX = 0;
	m_mouseRawY = 0;
	m_mouseLButton = false;
	m_mouseMButton = false;
	m_mouseRButton = false;
	m_mouseX1Button = false;
	m_mouseX2Button = false;

	for (size_t i = 0; i < MAX_CONTROLLERS; i++) {
		m_controllers[i].vibrateTimeLeft = 0;
		m_controllers[i].vibrateTimeRight = 0;
	}
}

InputManager::~InputManager()
{
	Safe_Delete_Map(m_mapKey);
}

bool InputManager::Init(HWND hWnd, bool capture)
{
	try {
		m_mouseCaptured = capture;

		m_Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		m_Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		m_Rid[0].dwFlags = RIDEV_INPUTSINK;

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

		if (!RegisterRawInputDevices(m_Rid, 1, sizeof(RAWINPUTDEVICE)))
		{
			MessageBox(nullptr, L"RegisterRawInputDevices error", L"RegisterRawInputDevices", MB_OK);
		}

		if (m_mouseCaptured) {
			SetCapture(hWnd);
		}

		// 컨트롤러 상태를 지운다.
		ZeroMemory(m_controllers, sizeof(ControllerState) * MAX_CONTROLLERS);
		CheckControllers();
	}
	catch (...) {
		ErrorLogger::Log("Input::Init()");
		return false;
	}

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

void InputManager::Clear(UCHAR what) {
	if (what & NSInput::KEYS_DOWN) { // KeysDown을 지우고 싶다면
		for (size_t i = 0; i < NSInput::KEYS_ARRAY_LEN; i++) {
			m_keysDown[i] = false;
		}
	}
	if (what & NSInput::KEYS_PRESSED) { // KeysPressed을 지우고 싶다면
		for (size_t i = 0; i < NSInput::KEYS_ARRAY_LEN; i++) {
			m_keysPressed[i] = false;
		}
	}
	if (what & NSInput::MOUSE) {
		m_mouseX = 0;
		m_mouseY = 0;
		m_mouseRawX = 0;
		m_mouseRawY = 0;
	}
	if (what & NSInput::TEXT_IN) {
		ClearTextIn();
	}
}

void InputManager::MouseIn(LPARAM lParam) {
	m_mouseX = GET_X_LPARAM(lParam);
	m_mouseY = GET_Y_LPARAM(lParam);

	SetCapture(GET_SINGLE(Core)->GetMainWnd());
}

void InputManager::MouseUp()
{
	ReleaseCapture();
}

void InputManager::MouseRawIn(LPARAM lParam) {
	/* 이 부분 RAWINPUT가 아니라 BYTE로 받으면 lastX,Y 데이터가 안 들어오는 점을 확인 */
	RAWINPUT input;
	memset(&input, 0, sizeof(input));

	UINT nSize = sizeof(RAWINPUT);

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &input, &nSize, sizeof(RAWINPUTHEADER));

	if (input.header.dwType == RIM_TYPEMOUSE)
	{
		m_mouseRawX = input.data.mouse.lLastX;
		m_mouseRawY = input.data.mouse.lLastY;
	}
}

void InputManager::MouseWheelIn(WPARAM wParam) {
	m_mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
}

void InputManager::CheckControllers() {
	DWORD result;
	for (DWORD i = 0; i < MAX_CONTROLLERS; i++)
	{
		result = XInputGetState(i, &m_controllers[i].state);
		if (result == ERROR_SUCCESS)
			m_controllers[i].connected = true;
		else
			m_controllers[i].connected = false;
	}
}

void InputManager::ReadControllers() {
	DWORD result;
	for (DWORD i = 0; i < MAX_CONTROLLERS; i++)
	{
		if (m_controllers[i].connected)
		{
			result = XInputGetState(i, &m_controllers[i].state);
			if (result == ERROR_DEVICE_NOT_CONNECTED)    // 컨트롤러가 접속이 끊겼다면
				m_controllers[i].connected = false;
		}
	}
}

BYTE InputManager::GetGamepadLeftTrigger(UINT n) {
	BYTE value = GetGamepadLeftTriggerUndead(n);
	if (value > triggerDeadzone)
		value = (value - triggerDeadzone) * 255 /
		(255 - triggerDeadzone);
	else
		value = 0;
	return value;
}

BYTE InputManager::GetGamepadRightTrigger(UINT n) {
	BYTE value = GetGamepadRightTriggerUndead(n);
	if (value > triggerDeadzone)
		value = (value - triggerDeadzone) * 255 /
		(255 - triggerDeadzone);
	else
		value = 0;
	return value;
}

SHORT InputManager::GetGamepadThumbLX(UINT n) {
	int x = GetGamepadThumbLXUndead(n);
	if (x > thumbstickDeadzone)
		x = (x - thumbstickDeadzone) * 32767 /
		(32767 - thumbstickDeadzone);
	else if (x < -thumbstickDeadzone)
		x = (x + thumbstickDeadzone) * 32767 /
		(32767 - thumbstickDeadzone);
	else
		x = 0;
	return static_cast<SHORT>(x);
}

SHORT InputManager::GetGamepadThumbLY(UINT n) {
	int y = GetGamepadThumbLYUndead(n);
	if (y > thumbstickDeadzone)
		y = (y - thumbstickDeadzone) * 32767 /
		(32767 - thumbstickDeadzone);
	else if (y < -thumbstickDeadzone)
		y = (y + thumbstickDeadzone) * 32767 /
		(32767 - thumbstickDeadzone);
	else
		y = 0;
	return static_cast<SHORT>(y);
}

SHORT InputManager::GetGamepadThumbRX(UINT n) {
	int x = GetGamepadThumbRXUndead(n);
	if (x > thumbstickDeadzone)
		x = (x - thumbstickDeadzone) * 32767 /
		(32767 - thumbstickDeadzone);
	else if (x < -thumbstickDeadzone)
		x = (x + thumbstickDeadzone) * 32767 /
		(32767 - thumbstickDeadzone);
	else
		x = 0;
	return static_cast<SHORT>(x);
}

SHORT InputManager::GetGamepadThumbRY(UINT n) {
	int y = GetGamepadThumbRYUndead(n);
	if (y > thumbstickDeadzone)
		y = (y - thumbstickDeadzone) * 32767 /
		(32767 - thumbstickDeadzone);
	else if (y < -thumbstickDeadzone)
		y = (y + thumbstickDeadzone) * 32767 /
		(32767 - thumbstickDeadzone);
	else
		y = 0;
	return static_cast<SHORT>(y);
}

void InputManager::VibrateControllers(float frameTime) {
	for (int i = 0; i < MAX_CONTROLLERS; i++)
	{
		if (m_controllers[i].connected)
		{
			m_controllers[i].vibrateTimeLeft -= frameTime;
			if (m_controllers[i].vibrateTimeLeft < 0)
			{
				m_controllers[i].vibrateTimeLeft = 0;
				m_controllers[i].vibration.wLeftMotorSpeed = 0;
			}
			m_controllers[i].vibrateTimeRight -= frameTime;
			if (m_controllers[i].vibrateTimeRight < 0)
			{
				m_controllers[i].vibrateTimeRight = 0;
				m_controllers[i].vibration.wRightMotorSpeed = 0;
			}
			XInputSetState(i, &m_controllers[i].vibration);
		}
	}
}