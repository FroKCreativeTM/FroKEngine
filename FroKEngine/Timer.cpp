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

	// �ý����� ���۵� ���� ���� �ð��� �и�������(millisecond) ������ �˷���
	// �̸� countsPerSec�� ������
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	m_SecondsPerCount = 1.0 / (double)countsPerSec;

    m_hWnd = hWnd;

    return true;
}

void Timer::Update()
{
	// Ÿ�̸Ӱ� �����ִٸ� �ð��� �� �ʿ䰡 ����.
	if (m_bStopped)
	{
		m_fDeltaTime = 0.0f;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	m_CurrTime = currTime;

	// ���� �ð��� ���� �ð����� ���� �̿� 1�ʴ� Ŭ�� ���� ���Ѵ�.
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
	// �츮�� ����ٸ�, ���� �� ����� �ð��� ���� ���ʽÿ�.
	// ���� ������ �̹� �Ͻ� ������ �־��ٸ� 
	// mStopTime - mBaseTime ���� ���̿��� �Ͻ� ������ �ð��� ���ԵǾ� �����Ƿ� ������� �ʽ��ϴ�.
	// �̸� �����ϱ� ���� mStopTime���� �Ͻ� ������ �ð��� �� �� �ֽ��ϴ�.
	//
	//                     |<--paused time-->|
	// ----*---------------*-----------------*------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mStopTime    mCurrTime

	if (m_bStopped)
	{
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}

	// mCurrTime - mBaseTime�� ���ݿ��� �Ͻ� ���� �ð��� ���ԵǸ�,
	// �츮�� ����ϰ� ���� �ʽ��ϴ�. �̸� �����ϱ� ���� ������ �� �� �ֽ��ϴ�.
	// mCurrTime���� �Ͻ� ������ �ð� : 
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
