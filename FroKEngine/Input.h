#pragma once

#define WIN32_LEAN_AND_MEAN
#include "Game.h"

// ������ ���콺�� ���� ����
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

/* ============================== ���� �е� ���� ���� ============================== */
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

// ���� Ű�� ���� �����ΰ�
// ���� ���� ������ �����ϴ� ���� Ű�� ������ �ִ� �����ΰ��� �����Ѵ�.
class Input
{

public:

    bool Init(HWND hwnd, bool capture);

    /* ============================ Ű���� ���� �޼��� ============================ */
    void KeyDown(WPARAM wParam);
    void KeyUp(WPARAM wParam);
    void KeyIn(WPARAM wParam);
    bool IsKeyDown(UCHAR vkey) const;
    bool WasKeyPressed(UCHAR vkey) const;
    bool AnyKeyPressed() const;
    void ClearKeyPress(UCHAR vkey);

    /* ============================ �ؽ�Ʈ ���� �޼��� ============================ */
    void Clear(UCHAR what);
    void ClearAll() { Clear(NSInput::KEYS_MOUSE_TEXT); }
    void ClearTextIn() { m_textIn.clear(); }
    void ClearCharIn() { m_charIn = 0; }
    std::string GetTextIn() { return m_textIn; }
    void SetTextIn(std::string str) { m_textIn = str; }
    char GetCharIn() { return m_charIn; }

    /* ============================ ���콺 ���� �޼��� ============================ */
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

    /* ============================ ���� �е� ���� �޼��� ============================ */

    // ��Ʈ�ѷ� ���� ���¸� üũ�Ѵ�.
    void CheckControllers();

    // �����е��� �Է��� �����Ѵ�.
    void ReadControllers();

    // thumbstick deadzone�� �����Ѵ�.
    void SetThumbstickDeadzone(short dz) { thumbstickDeadzone = abs(dz); }

    // trigger deadzone�� �����Ѵ�.
    void SetTriggerDeadzone(BYTE dz) { triggerDeadzone = dz; }

    // thumbstick deadzone�� �����´�.
    short GetThumbstickDeadzone() { return thumbstickDeadzone; }

    // trigger deadzone�� �����´�.
    BYTE GetTriggerDeadzone() { return static_cast<BYTE>(triggerDeadzone); }

    // Ư�� ���� ��Ʈ�ѷ��� ���¸� �����´�.
    const ControllerState* GetControllerState(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return &m_controllers[n];
    }

    // Ư�� ���� �е��� ���� ���¸� Ȯ���Ѵ�.
    bool GetGamepadConnected(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].connected;
    }

    // n��° ���� ��ư�� ���� ���¸� ��ȯ�Ѵ�.
    const WORD GetGamepadButtons(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.wButtons;
    }

    // ���� �е��� D�е尡 �� ��ư�� Ȯ���Ѵ�.
    bool GetGamepadDPadUp(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return ((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_DPAD_UP) != 0);
    }

    // ���� �е��� D�е尡 �Ʒ� ��ư�� Ȯ���Ѵ�.
    bool GetGamepadDPadDown(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return ((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_DPAD_DOWN) != 0);
    }

    // D �е��� ������ Ȯ���Ѵ�.
    bool GetGamepadDPadLeft(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return ((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_DPAD_LEFT) != 0);
    }

    // D �е��� �������� Ȯ���Ѵ�.
    bool GetGamepadDPadRight(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_DPAD_RIGHT) != 0);
    }

    // ��ŸƮ ��ư�� Ȯ���Ѵ�.
    bool GetGamepadStart(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_START_BUTTON) != 0);
    }

    // �� ��ư�� ���ȴ��� Ȯ���Ѵ�.
    bool GetGamepadBack(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_BACK_BUTTON) != 0);
    }

    // ���� �潺ƽ�� ���¸� ��ȯ�Ѵ�.
    bool GetGamepadLeftThumb(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_LEFT_THUMB) != 0);
    }

    // ������ �潺ƽ�� ���¸� ��ȯ�Ѵ�.
    bool GetGamepadRightThumb(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_RIGHT_THUMB) != 0);
    }

    // ���� ��� ��ư ���¸� ��ȯ�Ѵ�.
    bool GetGamepadLeftShoulder(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_LEFT_SHOULDER) != 0);
    }

    // ������ ��� ��ư ���¸� ��ȯ�Ѵ�.
    bool GetGamepadRightShoulder(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_RIGHT_SHOULDER) != 0);
    }

    // A ��ư ���¸� ��ȯ�Ѵ�.
    bool GetGamepadA(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_A) != 0);
    }

    // B ��ư ���¸� ��ȯ�Ѵ�.
    bool GetGamepadB(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_B) != 0);
    }

    // X ��ư ���¸� ��ȯ�Ѵ�.
    bool GetGamepadX(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_X) != 0);
    }

    // Y ��ư ���¸� ��ȯ�Ѵ�.
    bool GetGamepadY(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return bool((m_controllers[n].state.Gamepad.wButtons & GAMEPAD_Y) != 0);
    }

    // ���� �е��� ���� Ʈ���� ���� �����´�.
    // �� �� 0~255 ������ ������ ��ȯ�ȴ�.
    BYTE GetGamepadLeftTrigger(UINT n);

    // �������� �������� ���� ���� Ʈ���� ���� �����´�.
    BYTE GetGamepadLeftTriggerUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.bLeftTrigger;
    }

    // ���� �е��� ������ Ʈ���� ���� �����´�.
    // �� �� 0~255 ������ ������ ��ȯ�ȴ�.
    BYTE GetGamepadRightTrigger(UINT n);

    // �������� �������� ���� ������ Ʈ���� ���� �����´�.
    BYTE GetGamepadRightTriggerUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.bRightTrigger;
    }

    // ���� �潺ƽ X ���� ��ȯ�Ѵ�.
    // -32768 ~ 32767
    SHORT GetGamepadThumbLX(UINT n);

    // �������� ������� ���� ���� �潺ƽ X ���� ��ȯ�Ѵ�.
    SHORT GetGamepadThumbLXUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.sThumbLX;
    }


    // ���� �潺ƽ Y ���� ��ȯ�Ѵ�.
    // -32768 ~ 32767
    SHORT GetGamepadThumbLY(UINT n);

    // �������� ������� ���� ���� �潺ƽ �� ���� ��ȯ�Ѵ�.
    SHORT GetGamepadThumbLYUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.sThumbLY;
    }

    // ������ �潺ƽ X ���� ��ȯ�Ѵ�.
    // -32768 ~ 32767
    SHORT GetGamepadThumbRX(UINT n);

    // �������� ������� ���� ������ �潺ƽ X ���� ��ȯ�Ѵ�.
    SHORT GetGamepadThumbRXUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)   // if invalid controller number
            n = MAX_CONTROLLERS - 1;    // force valid
        return m_controllers[n].state.Gamepad.sThumbRX;
    }

    // ������ �潺ƽ X ���� ��ȯ�Ѵ�.
    // -32768 ~ 32767
    SHORT GetGamepadThumbRY(UINT n);

    // �������� ������� ���� ������ �潺ƽ Y ���� ��ȯ�Ѵ�.
    SHORT GetGamepadThumbRYUndead(UINT n)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        return m_controllers[n].state.Gamepad.sThumbRY;
    }

    // ���� ������ �����Ѵ�.
    // speed 0=off, 65536=100
    // sec�� 1�ʴ� �󸶳� ������ ���ΰ�.
    void GamePadVibrateLeft(UINT n, WORD speed, float sec)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        m_controllers[n].vibration.wLeftMotorSpeed = speed;
        m_controllers[n].vibrateTimeLeft = sec;
    }

    // ������ ������ �����Ѵ�.
    // speed 0=off, 65536=100
    // sec�� 1�ʴ� �󸶳� ������ ���ΰ�.
    void GamePadVibrateRight(UINT n, WORD speed, float sec)
    {
        if (n > MAX_CONTROLLERS - 1)
            n = MAX_CONTROLLERS - 1;
        m_controllers[n].vibration.wRightMotorSpeed = speed;
        m_controllers[n].vibrateTimeRight = sec;
    }

    void VibrateControllers(float frameTime);

private:
    bool m_keysDown[NSInput::KEYS_ARRAY_LEN];		// Ư�� Ű�� ���� ��� true�� �ȴ�.
    bool m_keysPressed[NSInput::KEYS_ARRAY_LEN];	// ���� ������ �����ϴ� ���� Ư�� Ű�� ���� �ִ� ��� true�� �ȴ�.

    std::string m_textIn;			// ����ڰ� �Է��� �ؽ�Ʈ
    char m_charIn;					// ���������� �Է��� ����.
    bool m_newLine;					// ���ο� ���� ���۵� �� true�� �ȴ�.
    int m_mouseX, m_mouseY;			// ȭ��� ���콺 ������
    int m_mouseRawX, m_mouseRawY;	// ������ ���콺 ������
    RAWINPUTDEVICE m_Rid[1];		// ������ ���콺��
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

