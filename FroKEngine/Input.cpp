#include "Input.h"
#include "Core.h"

DEFINITION_SINGLE(Input);

Input::Input() {
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

Input::~Input() {
	if (m_mouseCaptured) {
		ReleaseCapture();
	}
}

bool Input::Init(HWND hwnd, bool capture) {
	try {
		m_mouseCaptured = capture;

		m_Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		m_Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		m_Rid[0].dwFlags = RIDEV_INPUTSINK;
		m_Rid[0].hwndTarget = hwnd;
		if (!RegisterRawInputDevices(m_Rid, 1, sizeof(RAWINPUTDEVICE)))
		{
			MessageBox(nullptr, L"RegisterRawInputDevices error", L"RegisterRawInputDevices", MB_OK);
		}

		if (m_mouseCaptured) {
			SetCapture(hwnd);
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

void Input::KeyDown(WPARAM wParam) {
	// 범위 확인을 한다.
	if (wParam < NSInput::KEYS_ARRAY_LEN) {
		m_keysDown[wParam] = true;		// KeysDown 배열 갱신
		m_keysPressed[wParam] = true;	// KeysPressed 배열 갱신
	}
}

void Input::KeyUp(WPARAM wParam) {
	if (wParam < NSInput::KEYS_ARRAY_LEN) {
		m_keysDown[wParam] = false;		// KeysDown 배열 갱신
	}
}

void Input::KeyIn(WPARAM wParam) {
	if (m_newLine) {
		m_textIn.clear();
		m_newLine = false;
	}
	if (wParam == '\b') { // 백스페이스인 경우
		if (m_textIn.length() > 0) { // 문자가 존재한다면
			m_textIn.erase(m_textIn.size() - 1); // 마지막에 입력한 문자를 지운다.
		}
	}
	else {
		m_textIn += wParam;
		m_charIn = wParam;
	}

	if ((char)wParam == '\r') {
		m_newLine = true;
	}
}

bool Input::IsKeyDown(UCHAR vkey) const {
	if (vkey < NSInput::KEYS_ARRAY_LEN) {
		return m_keysDown[vkey];
	}
	else {
		return false;
	}
}

bool Input::WasKeyPressed(UCHAR vkey) const {
	if (vkey < NSInput::KEYS_ARRAY_LEN) {
		return m_keysPressed[vkey];
	}
	else {
		return false;
	}
}

bool Input::AnyKeyPressed() const {
	for (size_t i = 0; i < NSInput::KEYS_ARRAY_LEN; i++)
	{
		if (m_keysPressed[i] == true) {
			return true;
		}
	}
	return false;
}

void Input::ClearKeyPress(UCHAR vkey) {
	if (vkey < NSInput::KEYS_ARRAY_LEN) {
		m_keysPressed[vkey] = false;
	}
}

void Input::Clear(UCHAR what) {
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

void Input::MouseIn(LPARAM lParam) {
	m_mouseX = GET_X_LPARAM(lParam);
	m_mouseY = GET_Y_LPARAM(lParam);

	SetCapture(Core::GetInst()->GetMainWnd());
}

void Input::MouseUp()
{
	ReleaseCapture();
}

void Input::MouseRawIn(LPARAM lParam) {
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

void Input::MouseWheelIn(WPARAM wParam) {
	m_mouseWheel = GET_WHEEL_DELTA_WPARAM(wParam);
}

void Input::CheckControllers() {
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

void Input::ReadControllers() {
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

BYTE Input::GetGamepadLeftTrigger(UINT n) {
	BYTE value = GetGamepadLeftTriggerUndead(n);
	if (value > triggerDeadzone)
		value = (value - triggerDeadzone) * 255 /
		(255 - triggerDeadzone);
	else
		value = 0;
	return value;
}

BYTE Input::GetGamepadRightTrigger(UINT n) {
	BYTE value = GetGamepadRightTriggerUndead(n);
	if (value > triggerDeadzone)
		value = (value - triggerDeadzone) * 255 /
		(255 - triggerDeadzone);
	else
		value = 0;
	return value;
}

SHORT Input::GetGamepadThumbLX(UINT n) {
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

SHORT Input::GetGamepadThumbLY(UINT n) {
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

SHORT Input::GetGamepadThumbRX(UINT n) {
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

SHORT Input::GetGamepadThumbRY(UINT n) {
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

void Input::VibrateControllers(float frameTime) {
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
