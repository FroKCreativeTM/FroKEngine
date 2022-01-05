#pragma once

#include "Game.h"

class Timer
{
public:
	bool Init(HWND hWnd);
	void Update();

	float GetTotalTime() const; // in seconds
	float GetDeltaTime() const; // in seconds

	void Reset();
	void Stop();
	void Start();

	DECLARE_SINGLE(Timer)

private:
	HWND			m_hWnd;

	double m_SecondsPerCount;
	double m_fDeltaTime;

	__int64 m_BaseTime;
	__int64 m_PausedTime;
	__int64 m_StopTime;
	__int64 m_PrevTime;
	__int64 m_CurrTime;

	bool m_bStopped;
};

