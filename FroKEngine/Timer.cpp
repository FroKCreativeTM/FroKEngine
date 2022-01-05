#include "Timer.h"

DEFINITION_SINGLE(Timer)

Timer::Timer()
	: m_SecondsPerCount(0.0), m_fDeltaTime(-1.0), m_BaseTime(0),
	m_PausedTime(0), m_PrevTime(0), m_CurrTime(0), m_bStopped(false)
{
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0 / (double)countsPerSec;
}

Timer::~Timer() {}

bool Timer::Init(HWND hWnd)
{
	__int64 countsPerSec;

	// 시스템이 시작된 이후 지난 시간을 밀리세컨드(millisecond) 단위로 알려줌
	// 이를 countsPerSec에 저장함
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0 / (double)countsPerSec;

    m_hWnd = hWnd;

    return true;
}

void Timer::Update()
{
	// 타이머가 멈춰있다면 시간이 갈 필요가 없다.
	if (m_bStopped)
	{
		m_fDeltaTime = 0.0f;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurrTime = currTime;

	// 현재 시간을 이전 시간으로 빼고 이에 1초당 클락 수로 곱한다.
	m_fDeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;

	// Prepare for next frame.
	m_PrevTime = m_CurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if (m_fDeltaTime < 0.0)
	{
		m_fDeltaTime = 0.0;
	}
}

float Timer::GetTotalTime() const
{
	// 우리가 멈췄다면, 멈춘 후 경과한 시간을 세지 마십시오.
	// 또한 이전에 이미 일시 중지가 있었다면 
	// mStopTime - mBaseTime 간격 사이에는 일시 중지된 시간이 포함되어 있으므로 계산하지 않습니다.
	// 이를 수정하기 위해 mStopTime에서 일시 중지된 시간을 뺄 수 있습니다.
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (m_bStopped)
	{
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}

	// mCurrTime - mBaseTime의 간격에는 일시 중지 시간이 포함되며,
	// 우리는 계산하고 싶지 않습니다. 이를 수정하기 위해 다음을 뺄 수 있습니다.
	// mCurrTime에서 일시 중지된 시간 : 
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime

	else
	{
		return (float)(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}
}

float Timer::GetDeltaTime() const
{
	return (float)m_fDeltaTime;
}

void Timer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_bStopped = false;
}

void Timer::Stop()
{
	if (!m_bStopped)
	{
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		m_StopTime = currTime;
		m_bStopped = true;
	}
}

void Timer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if (m_bStopped)
	{
		m_PausedTime += (startTime - m_StopTime);
		 
		m_PrevTime = startTime;
		m_StopTime = 0;
		m_bStopped = false;
	}
}
