#pragma once

#define WIN32_LEAN_AND_MEAN
#include "Game.h"

// 고정밀 마우스를 위한 설정
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC      ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE     ((USHORT) 0x02)
#endif
//--------------------------

namespace NSInput
{
    const int KEYS_ARRAY_LEN = 256;

    const UCHAR KEYS_DOWN = 1;
    const UCHAR KEYS_PRESSED = 2;
    const UCHAR MOUSE = 4;
    const UCHAR TEXT_IN = 8;
    const UCHAR KEYS_MOUSE_TEXT = KEYS_DOWN + KEYS_PRESSED + MOUSE + TEXT_IN;
}

/* ============================== 게임 패드 관련 변수 ============================== */
const short GAMEPAD_THUMBSTICK_DEADZONE = (short)(0.20f * 0X7FFF);
const short GAMEPAD_TRIGGER_DEADZONE = 20;
const DWORD MAX_CONTROLLERS = 4;

// Bit corresponding to gamepad button in state.Gamepad.wButtons
const DWORD GAMEPAD_DPAD_UP = 0x0001;
const DWORD GAMEPAD_DPAD_DOWN = 0x0002;
const DWORD GAMEPAD_DPAD_LEFT = 0x0004;
const DWORD GAMEPAD_DPAD_RIGHT = 0x0008;
const DWORD GAMEPAD_START_BUTTON = 0x0010;
const DWORD GAMEPAD_BACK_BUTTON = 0x0020;
const DWORD GAMEPAD_LEFT_THUMB = 0x0040;
const DWORD GAMEPAD_RIGHT_THUMB = 0x0080;
const DWORD GAMEPAD_LEFT_SHOULDER = 0x0100;
const DWORD GAMEPAD_RIGHT_SHOULDER = 0x0200;
const DWORD GAMEPAD_A = 0x1000;
const DWORD GAMEPAD_B = 0x2000;
const DWORD GAMEPAD_X = 0x4000;
const DWORD GAMEPAD_Y = 0x8000;

struct ControllerState
{
    XINPUT_STATE        state;
    XINPUT_VIBRATION    vibration;
    float               vibrateTimeLeft;    // mSec
    float               vibrateTimeRight;   // mSec
    bool                connected;
};

// 현재 키가 누른 상태인가
// 현재 게임 루프가 동작하는 동안 키를 누르고 있는 상태인가를 저장한다.
class Input
{

public:

    bool Init(HWND hwnd, bool capture);

    /* ============================ 키보드 관련 메서드 ============================ */
    void KeyDown(WPARAM wParam);
    void KeyUp(WPARAM wParam);
    void KeyIn(WPARAM wParam);
    bool IsKeyDown(UCHAR vkey) const;
    bool WasKeyPressed(UCHAR vkey) const;
    bool AnyKeyPressed() const;
    void ClearKeyPress(UCHAR vkey);

    /* ============================ 텍스트 관련 메서드 ============================ */
    void Clear(UCHAR what);
    void ClearAll() { Clear(NSInput::KEYS_MOUSE_TEXT); }
    void ClearTextIn() { m_textIn.clear(); }
    void ClearCharIn() { m_charIn = 0; }
    std::string GetTextIn() { return m_textIn; }
    void SetTextIn(std::string str) { m_textIn = str; }
    char GetCharIn() { return m_charIn; }

    /* ============================ 마우스 관련 메서드 ============================ */
    void MouseIn(LPARAM lParam);
    void MouseUp();
    void MouseRawIn(LPARAM lParam);
    void MouseWheelIn(WPARAM wParam);
    void SetMouseLButton(bool b) { m_mouseLButton = b; }
    void SetMouseMButton(bool b) { m_mouseMButton = b; }
    void SetMouseRButton(bool b) { m_mouseRButton = b; }
    void SetMouseXButton(WPARAM wParam) {
        m_mouseX1Button = (wParam & MK_XBUTTON1) ? true : false;
        m_mouseX2Button = (wParam & MK_XBUTTON2) ? true : false;
    }

    int  GetMouseX() { return m_mouseX; }
    int  GetMouseY() { return m_mouseY; }

    int  GetMouseRawX() { return m_mouseRawX; }
    int  GetMouseRawY() { return m_mouseRawY; }

    int  GetMouseWheel() {
        int wheel = m_mouseWheel;
        m_mouseWheel = 0;
        return wheel;
    }

    bool GetMouseLButton()  const { return m_mouseLButton; }
    bool GetMouseMButton()  const { return m_mouseMButton; }
    bool GetMouseRButton()  const { return m_mouseRButton; }
    bool GetMouseX1Button() const { return m_mouseX1Button; }
    bool GetMouseX2Button() const { return m_mouseX2Button; }

    /* ============================ 게임 패드 관련 메서드 ============================ */

    // 컨트롤러 연결 상태를 체크한다.
    void CheckControllers();

    // 게임패드의 입력을 저장한다.
    void ReadControllers();

    // thumbstick deadzone을 설정한다.
    void SetThumbstickDeadzone(short dz) { thumbstickDeadzone = abs(dz); }

    // trigger deadzone을 설정한다.
    void SetTriggerDeadzone(BYTE dz) { triggerDeadzone = dz; }

    // thumbstick deadzone을 가져온다.
    short GetThumbstickDeadzone() { return thumbstickDeadzone; }

    // trigger deadzone을 가져온다.
    BYTE GetTriggerDeadzone() { return static_cast<BYTE>(triggerDeadzone); }

    // 특정 게임 컨트롤러의 상태를 가져온다.
    const ControllerState* GetControllerState(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return &m_controllers[n];
    }

    // 특정 게임 패드의 연결 상태를 확인한다.
    bool GetGamepadConnected(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].connected;
    }

    // n번째 게임 버튼의 눌림 상태를 반환한다.
    const WORD GetGamepadButtons(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.wButtons;
    }

    // 게임 패드의 D패드가 위 버튼을 확인한다.
    bool GetGamepadDPadUp(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return ((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_DPAD_UP) != 0);
    }

    // 게임 패드의 D패드가 아래 버튼을 확인한다.
    bool GetGamepadDPadDown(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return ((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_DPAD_DOWN) != 0);
    }

    // D 패드의 왼쪽을 확인한다.
    bool GetGamepadDPadLeft(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return ((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_DPAD_LEFT) != 0);
    }

    // D 패드의 오른쪽을 확인한다.
    bool GetGamepadDPadRight(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_DPAD_RIGHT) != 0);
    }

    // 스타트 버튼을 확인한다.
    bool GetGamepadStart(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_START_BUTTON) != 0);
    }

    // 백 버튼이 눌렸는지 확인한다.
    bool GetGamepadBack(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_BACK_BUTTON) != 0);
    }

    // 왼쪽 썸스틱의 상태를 반환한다.
    bool GetGamepadLeftThumb(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_LEFT_THUMB) != 0);
    }

    // 오른쪽 썸스틱의 상태를 반환한다.
    bool GetGamepadRightThumb(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_RIGHT_THUMB) != 0);
    }

    // 왼쪽 숄더 버튼 상태를 반환한다.
    bool GetGamepadLeftShoulder(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_LEFT_SHOULDER) != 0);
    }

    // 오른쪽 숄더 버튼 상태를 반환한다.
    bool GetGamepadRightShoulder(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_RIGHT_SHOULDER) != 0);
    }

    // A 버튼 상태를 반환한다.
    bool GetGamepadA(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_A) != 0);
    }

    // B 버튼 상태를 반환한다.
    bool GetGamepadB(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_B) != 0);
    }

    // X 버튼 상태를 반환한다.
    bool GetGamepadX(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_X) != 0);
    }

    // Y 버튼 상태를 반환한다.
    bool GetGamepadY(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_Y) != 0);
    }

    // 게임 패드의 왼쪽 트리거 값을 가져온다.
    // 이 때 0~255 상태의 값으로 반환된다.
    BYTE GetGamepadLeftTrigger(UINT n);

    // 데드존이 설정되지 않은 왼쪽 트리거 값을 가져온다.
    BYTE GetGamepadLeftTriggerUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.bLeftTrigger;
    }

    // 게임 패드의 오른쪽 트리거 값을 가져온다.
    // 이 때 0~255 상태의 값으로 반환된다.
    BYTE GetGamepadRightTrigger(UINT n);

    // 데드존이 설정되지 않은 오른쪽 트리거 값을 가져온다.
    BYTE GetGamepadRightTriggerUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.bRightTrigger;
    }

    // 왼쪽 썸스틱 X 값을 반환한다.
    // -32768 ~ 32767
    SHORT GetGamepadThumbLX(UINT n);

    // 데드존이 적용되지 않은 왼쪽 썸스틱 X 값을 반환한다.
    SHORT GetGamepadThumbLXUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.sThumbLX;
    }


    // 왼쪽 썸스틱 Y 값을 반환한다.
    // -32768 ~ 32767
    SHORT GetGamepadThumbLY(UINT n);

    // 데드존이 적용되지 않은 왼쪽 썸스틱 ㅛ 값을 반환한다.
    SHORT GetGamepadThumbLYUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.sThumbLY;
    }

    // 오른쪽 썸스틱 X 값을 반환한다.
    // -32768 ~ 32767
    SHORT GetGamepadThumbRX(UINT n);

    // 데드존이 적용되지 않은 오른쪽 썸스틱 X 값을 반환한다.
    SHORT GetGamepadThumbRXUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)   // if invalid controller number
            n = MAX_CONTROLLERS - 1;    // force valid
        return m_controllers[n].state.Gamepad.sThumbRX;
    }

    // 오른쪽 썸스틱 X 값을 반환한다.
    // -32768 ~ 32767
    SHORT GetGamepadThumbRY(UINT n);

    // 데드존이 적용되지 않은 오른쪽 썸스틱 Y 값을 반환한다.
    SHORT GetGamepadThumbRYUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.sThumbRY;
    }

    // 왼쪽 진동을 설정한다.
    // speed 0=off, 65536=100
    // sec는 1초당 얼마나 진동할 것인가.
    void GamePadVibrateLeft(UINT n, WORD speed, float sec)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        m_controllers[n].vibration.wLeftMotorSpeed = speed;
        m_controllers[n].vibrateTimeLeft = sec;
    }

    // 오른쪽 진동을 설정한다.
    // speed 0=off, 65536=100
    // sec는 1초당 얼마나 진동할 것인가.
    void GamePadVibrateRight(UINT n, WORD speed, float sec)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        m_controllers[n].vibration.wRightMotorSpeed = speed;
        m_controllers[n].vibrateTimeRight = sec;
    }

    void VibrateControllers(float frameTime);

private:
    bool m_keysDown[NSInput::KEYS_ARRAY_LEN];		// 특정 키를 누른 경우 true가 된다.
    bool m_keysPressed[NSInput::KEYS_ARRAY_LEN];	// 게임 루프가 동작하는 동안 특정 키를 누륵 있는 경우 true가 된다.

    std::string m_textIn;			// 사용자가 입력한 텍스트
    char m_charIn;					// 마지막으로 입력한 문자.
    bool m_newLine;					// 새로운 줄이 시작될 때 true가 된다.
    int m_mouseX, m_mouseY;			// 화면상 마우스 데이터
    int m_mouseRawX, m_mouseRawY;	// 고정밀 마우스 데이터
    RAWINPUTDEVICE m_Rid[1];		// 고정밀 마우스용
    bool m_mouseCaptured;
    bool m_mouseLButton;
    bool m_mouseMButton;
    bool m_mouseRButton;
    bool m_mouseX1Button;
    bool m_mouseX2Button;
    int  m_mouseWheel;
    ControllerState m_controllers[MAX_CONTROLLERS];

    short thumbstickDeadzone;
    short triggerDeadzone;

    DECLARE_SINGLE(Input)
};

